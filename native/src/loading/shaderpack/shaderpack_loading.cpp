/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#include "shaderpack_loading.hpp"
#include "../folder_accessor.hpp"
#include "../loading_utils.hpp"
#include "../zip_folder_accessor.hpp"
#include "../regular_folder_accessor.hpp"
#include "../utils.hpp"
#include "json_interop.hpp"

#include <future>

namespace nova {
    fs::path SHADERPACK_ROOT("shaderpacks");
    fs::path BEDROCK_SHADERPACK_ROOT("resourcepacks");

    folder_accessor_base* get_shaderpack_accessor(const fs::path &shaderpack_name);

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, void *arg);
    void load_passes_file(ftl::TaskScheduler *task_scheduler, void *arg);
    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, void *arg);
    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, void *arg);
    void load_material_files(ftl::TaskScheduler *task_scheduler, void *arg);
    void load_single_material(ftl::TaskScheduler *task_scheduler, void *arg);

    template<typename DataType>
    struct load_data_args {
        folder_accessor_base* folder_access;
        std::promise<DataType> promise;
    };

    shaderpack_data load_shaderpack_data(const fs::path& shaderpack_name, ftl::TaskScheduler& task_scheduler) {
        folder_accessor_base* folder_access = get_shaderpack_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem
        
        // Load resource definitions
        auto load_resources_data = load_data_args<shaderpack_resources_data>{folder_access, std::promise<shaderpack_resources_data>()};
        ftl::Task load_dynamic_resource_task = { load_dynamic_resources_file, &load_resources_data };
        task_scheduler.AddTask(load_dynamic_resource_task);

        // Load pass definitions
        auto load_passes_data = load_data_args<std::vector<render_pass_data>>{folder_access, std::promise<std::vector<render_pass_data>>()};
        ftl::Task load_passes_task = { load_passes_file, &load_passes_data };
        task_scheduler.AddTask(load_passes_task);

        // Load pipeline definitions
        auto load_pipelines_data = load_data_args<std::vector<pipeline_data>>{folder_access, std::promise<std::vector<pipeline_data>>()};
        ftl::Task load_pipelines_task = { load_pipeline_files, &load_pipelines_data };
        task_scheduler.AddTask(load_pipelines_task);

        auto load_materials_data = load_data_args<std::vector<material_data>>{ folder_access, std::promise<std::vector<material_data>>() };
        ftl::Task load_materials_task = { load_material_files, &load_materials_data };
        task_scheduler.AddTask(load_materials_task);

        shaderpack_data data;
        data.pipelines = load_pipelines_data.promise.get_future().get();
        data.passes = load_passes_data.promise.get_future().get();
        data.resources = load_resources_data.promise.get_future().get();

        delete folder_access;

        return data;
    }

    folder_accessor_base* get_shaderpack_accessor(const fs::path &shaderpack_name) {
        folder_accessor_base* folder_access = nullptr;
        fs::path path_to_shaderpack = SHADERPACK_ROOT / shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            folder_access = new zip_folder_accessor(path_to_shaderpack);

        } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            folder_access = new regular_folder_accessor(path_to_shaderpack);

        } else {
            path_to_shaderpack = BEDROCK_SHADERPACK_ROOT / shaderpack_name;

            if(is_zip_folder(path_to_shaderpack)) {
                // zip folder in the resourcepacks folder
                path_to_shaderpack.replace_extension(".zip");
                folder_access = new zip_folder_accessor(path_to_shaderpack);

            } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
                folder_access = new regular_folder_accessor(path_to_shaderpack);
            }
        }

        if(folder_access == nullptr) {
            throw resource_not_found_error(shaderpack_name.string());
        }

        return folder_access;
    }

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = reinterpret_cast<load_data_args<shaderpack_resources_data>*>(arg);
        try {
            auto resources_bytes = args->folder_access->read_resource("resources.json");
            auto json_resource = nlohmann::json::parse(resources_bytes);
            auto resources = json_resource.get<shaderpack_resources_data>();
            args->promise.set_value(resources);

        } catch(resource_not_found_error& err) {
            // No resources defined.. I guess they think they don't need any?
            args->promise.set_value({});
        }
    }

    void load_passes_file(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = reinterpret_cast<load_data_args<std::vector<render_pass_data>>*>(arg);
        auto passes_bytes = args->folder_access->read_resource("passes.json");
        auto json_passes = nlohmann::json::parse(passes_bytes);
        auto passes = json_passes.get<std::vector<render_pass_data>>();
        args->promise.set_value(passes);
    }

    struct load_pipeline_data {
        folder_accessor_base* folder_access;
        std::mutex& mut;
        std::vector<std::promise<pipeline_data>>& output;
        size_t& cur_promise;
        const fs::path& pipeline_path;
    };

    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = reinterpret_cast<load_data_args<std::vector<pipeline_data>>*>(arg);

        auto potential_pipeline_files = args->folder_access->get_all_items_in_folder("materials");

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        std::vector<std::promise<pipeline_data>> pipeline_data_promises;
        pipeline_data_promises.resize(potential_pipeline_files.size());
        size_t cur_promise = 0;
        std::mutex promises_mutex;

        uint32_t num_pipelines = 0;

        for(const fs::path& potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                auto data_for_loading_pipeline = load_pipeline_data{args->folder_access, promises_mutex, pipeline_data_promises,
                                                                    cur_promise, potential_file};
                ftl::Task load_single_pipeline_task = { load_single_pipeline, &data_for_loading_pipeline };
                task_scheduler->AddTask(load_single_pipeline_task);
            }
        }

        std::vector<pipeline_data> pipelines;
        pipelines.resize(num_pipelines);
        for(uint32_t i = 0; i < num_pipelines; i++) {
            pipelines.push_back(pipeline_data_promises.at(i).get_future().get());
        }
        args->promise.set_value(pipelines);
    }

    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = reinterpret_cast<load_pipeline_data*>(arg);
        auto pipeline_bytes = args->folder_access->read_resource(args->pipeline_path);
        auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
        auto pipeline = json_pipeline.get<pipeline_data>();
        std::lock_guard<std::mutex> promises_lock(args->mut);
        args->output.at(args->cur_promise).set_value(pipeline);
        args->cur_promise++;
    }

    struct load_material_data {
        folder_accessor_base* folder_access;
        std::mutex& mut;
        std::vector<std::promise<material_data>>& output;
        size_t& cur_promise;
        const fs::path& material_path;
    };

    void load_material_files(ftl::TaskScheduler * task_scheduler, void * arg) {
        auto* args = reinterpret_cast<load_data_args<std::vector<material_data>>*>(arg);

        auto potential_material_files = args->folder_access->get_all_items_in_folder("materials");

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        std::vector<std::promise<material_data>> material_data_promises;
        material_data_promises.resize(potential_material_files.size());
        size_t cur_promise = 0;
        std::mutex promises_mutex;

        uint32_t num_materials = 0;

        for(const fs::path& potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                // Pipeline file!
                auto data_for_loading_material = load_material_data{ args->folder_access, promises_mutex, material_data_promises,
                    cur_promise, potential_file };
                ftl::Task load_single_material_task = { load_single_material, &data_for_loading_material };
                task_scheduler->AddTask(load_single_material_task);
            }
        }

        std::vector<material_data> materials;
        materials.resize(num_materials);
        for(uint32_t i = 0; i < num_materials; i++) {
            materials.push_back(material_data_promises.at(i).get_future().get());
        }
        args->promise.set_value(materials);
    }

    void load_single_material(ftl::TaskScheduler * task_scheduler, void * arg) {
        auto* args = reinterpret_cast<load_material_data*>(arg);
        auto material_bytes = args->folder_access->read_resource(args->material_path);
        auto json_material = nlohmann::json::parse(material_bytes);
        auto material = json_material.get<material_data>();
        std::lock_guard<std::mutex> promises_lock(args->mut);
        args->output.at(args->cur_promise).set_value(material);
        args->cur_promise++;
    }
}
