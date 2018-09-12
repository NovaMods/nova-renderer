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
#include <ftl/atomic_counter.h>

namespace nova {
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
        DataType output;
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
        
        ftl::AtomicCounter loading_tasks_remaining(&task_scheduler);

        // Load resource definitions
        auto *load_resources_data = new load_data_args<shaderpack_resources_data>{folder_access, shaderpack_resources_data()};
        ftl::Task load_dynamic_resource_task = { load_dynamic_resources_file, load_resources_data };
        task_scheduler.AddTask(load_dynamic_resource_task, &loading_tasks_remaining);

        // Load pass definitions
        auto *load_passes_data = new load_data_args<std::vector<render_pass_data>>{folder_access, std::vector<render_pass_data>()};
        ftl::Task load_passes_task = { load_passes_file, load_passes_data };
        task_scheduler.AddTask(load_passes_task, &loading_tasks_remaining);

        // Load pipeline definitions
        auto *load_pipelines_data = new load_data_args<std::vector<pipeline_data>>{folder_access, std::vector<pipeline_data>()};
        ftl::Task load_pipelines_task = { load_pipeline_files, load_pipelines_data };
        task_scheduler.AddTask(load_pipelines_task, &loading_tasks_remaining);

        auto *load_materials_data = new load_data_args<std::vector<material_data>>{ folder_access, std::vector<material_data>() };
        ftl::Task load_materials_task = { load_material_files, load_materials_data };
        task_scheduler.AddTask(load_materials_task, &loading_tasks_remaining);

        task_scheduler.WaitForCounter(&loading_tasks_remaining, 0);

        shaderpack_data data;
        data.pipelines = load_pipelines_data->output;
        data.passes = load_passes_data->output;
        data.resources = load_resources_data->output;
        data.materials = load_materials_data->output;

        delete folder_access;
        delete load_resources_data;
        delete load_passes_data;
        delete load_pipelines_data;
        delete load_materials_data;

        return data;
    }

    folder_accessor_base* get_shaderpack_accessor(const fs::path &shaderpack_name) {
        folder_accessor_base* folder_access = nullptr;
        fs::path path_to_shaderpack =  shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            folder_access = new zip_folder_accessor(path_to_shaderpack);

        } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            folder_access = new regular_folder_accessor(path_to_shaderpack);

        } else {
            path_to_shaderpack = shaderpack_name;

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
        auto* args = static_cast<load_data_args<shaderpack_resources_data>*>(arg);
        try {
            std::string resources_string = args->folder_access->read_text_file("resources.json");
            auto json_resource = nlohmann::json::parse(resources_string.c_str());
            auto resources = json_resource.get<shaderpack_resources_data>();
            args->output.samplers = std::move(resources.samplers);
            args->output.textures = std::move(resources.textures);

        } catch(resource_not_found_error&) {
            // No resources defined.. I guess they think they don't need any?
            NOVA_LOG(DEBUG) << "No resources file found for shaderpack";

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's resources.json: " << err.what();
        }
    }

    void load_passes_file(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = static_cast<load_data_args<std::vector<render_pass_data>>*>(arg);
        auto passes_bytes = args->folder_access->read_text_file("passes.json");
        try {
            auto json_passes = nlohmann::json::parse(passes_bytes);
            auto passes = json_passes.get<std::vector<render_pass_data>>();
            args->output = std::move(passes);

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's passes.json: " << err.what();
        }
    }

    struct load_pipeline_data {
        folder_accessor_base* folder_access;
        size_t out_idx;
        std::vector<pipeline_data>* output;
        const fs::path* pipeline_path;
    };

    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = static_cast<load_data_args<std::vector<pipeline_data>>*>(arg);

        std::vector<fs::path> potential_pipeline_files;
        try {
            potential_pipeline_files = args->folder_access->get_all_items_in_folder("materials");
        } catch (const filesystem_exception &exception) {
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        std::vector<pipeline_data> pipeline_data_promises;
        pipeline_data_promises.resize(potential_pipeline_files.size());

        uint32_t num_pipelines = 0;

        ftl::AtomicCounter pipeline_load_tasks_remaining(task_scheduler);

        std::vector<load_pipeline_data> datas;

        for(const fs::path& potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                load_pipeline_data data_for_loading_pipeline{};
                data_for_loading_pipeline.folder_access = args->folder_access;
                data_for_loading_pipeline.out_idx = num_pipelines;
                data_for_loading_pipeline.output = &pipeline_data_promises;
                data_for_loading_pipeline.pipeline_path = &potential_file;

                datas.emplace_back(data_for_loading_pipeline);

                ftl::Task load_single_pipeline_task = { load_single_pipeline, &datas[num_pipelines] };
                task_scheduler->AddTask(load_single_pipeline_task, &pipeline_load_tasks_remaining);
                num_pipelines++;
            }
        }

        task_scheduler->WaitForCounter(&pipeline_load_tasks_remaining, 0);

        args->output.resize(num_pipelines);
        for(uint32_t i = 0; i < num_pipelines; i++) {
            args->output.push_back(pipeline_data_promises.at(i));
        }
    }

    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, void *arg) {
        auto* args = static_cast<load_pipeline_data*>(arg);
        auto pipeline_bytes = args->folder_access->read_text_file(*args->pipeline_path);
        try {
            auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
            auto pipeline = json_pipeline.get<pipeline_data>();

            args->output->emplace(args->output->begin() + args->out_idx, pipeline);

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse pipeline file " << args->pipeline_path->string() << ": " << err.what();
        }
    }

    struct load_material_data {
        folder_accessor_base* folder_access;
        size_t out_idx;
        std::vector<material_data>* output;
        const fs::path* material_path;
    };

    void load_material_files(ftl::TaskScheduler * task_scheduler, void * arg) {
        auto* args = static_cast<load_data_args<std::vector<material_data>>*>(arg);


        std::vector<fs::path> potential_material_files;
        try {
            potential_material_files = args->folder_access->get_all_items_in_folder("materials");
        } catch (const filesystem_exception &exception) {
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        std::vector<material_data> loaded_material_data;
        loaded_material_data.resize(potential_material_files.size());
        size_t cur_promise = 0;
        ftl::AtomicCounter material_load_tasks_remaining(task_scheduler);

        uint32_t num_materials = 0;

        std::vector<load_material_data> datas;

        for(const fs::path& potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                // Material file!
                load_material_data data_for_loading_material;
                data_for_loading_material.folder_access = args->folder_access;
                data_for_loading_material.material_path = &potential_file;
                data_for_loading_material.out_idx = num_materials;
                data_for_loading_material.output = &loaded_material_data;

                datas.emplace_back(data_for_loading_material);

                ftl::Task load_single_material_task = { load_single_material, &datas[num_materials] };
                task_scheduler->AddTask(load_single_material_task, &material_load_tasks_remaining);

                num_materials++;
            }
        }

        task_scheduler->WaitForCounter(&material_load_tasks_remaining, 0);

        args->output.resize(num_materials);
        for(uint32_t i = 0; i < num_materials; i++) {
            args->output.push_back(loaded_material_data.at(i));
        }
    }

    void load_single_material(ftl::TaskScheduler * task_scheduler, void * arg) {
        auto* args = static_cast<load_material_data*>(arg);
        auto material_bytes = args->folder_access->read_text_file(*args->material_path);
        try {
            auto json_material = nlohmann::json::parse(material_bytes);
            auto material = json_material.get<material_data>();
            args->output->emplace(args->output->begin() + args->out_idx, material);

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse material file " << args->material_path->string() << ": " << err.what();
        }
    }
}
