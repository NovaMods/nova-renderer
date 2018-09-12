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

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, shaderpack_resources_data& output);
    void load_passes_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, std::vector<render_pass_data>& output);
    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, std::vector<pipeline_data>& output);
    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, const fs::path& pipeline_path, uint32_t out_idx, std::vector<pipeline_data>& output);
    void load_material_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, std::vector<material_data>& output);
    void load_single_material(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, const fs::path& material_path, uint32_t out_idx, std::vector<material_data>& output);

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

        shaderpack_data data;

        // Load resource definitions
        shaderpack_resources_data loaded_resources;
        task_scheduler.AddTask(&loading_tasks_remaining, load_dynamic_resources_file, folder_access, data.resources);

        // Load pass definitions
        std::vector<render_pass_data> loaded_passes;
        task_scheduler.AddTask(&loading_tasks_remaining, load_passes_file, folder_access, data.passes);

        // Load pipeline definitions
        std::vector<pipeline_data> loaded_pipelines;
        task_scheduler.AddTask(&loading_tasks_remaining, load_pipeline_files, folder_access, data.pipelines);

        std::vector<material_data> loaded_materials;
        task_scheduler.AddTask(&loading_tasks_remaining, load_material_files, folder_access, data.materials);

        task_scheduler.WaitForCounter(&loading_tasks_remaining, 0);
        
        delete folder_access;

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

            } else if(fs::exists(path_to_shaderpack)) {
                folder_access = new regular_folder_accessor(path_to_shaderpack);
            }
        }

        if(folder_access == nullptr) {
            throw resource_not_found_error(shaderpack_name.string());
        }

        return folder_access;
    }

    void load_dynamic_resources_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, shaderpack_resources_data& output) {
        std::string resources_string = folder_access->read_text_file("resources.json");
        try {
            auto json_resource = nlohmann::json::parse(resources_string.c_str());
            auto resources = json_resource.get<shaderpack_resources_data>();
            output.samplers = std::move(resources.samplers);
            output.textures = std::move(resources.textures);

        } catch(resource_not_found_error&) {
            // No resources defined.. I guess they think they don't need any?
            NOVA_LOG(WARN) << "No resources file found for shaderpack";

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's resources.json: " << err.what();
        }
    }

    void load_passes_file(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, std::vector<render_pass_data>& output) {
        const auto passes_bytes = folder_access->read_text_file("passes.json");
        try {
            auto json_passes = nlohmann::json::parse(passes_bytes);
            auto passes = json_passes.get<std::vector<render_pass_data>>();
            output = std::move(passes);

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's passes.json: " << err.what();
        }

        // Don't check for a resources_not_found exception because a shaderpack _needs_ a passes.json and if the 
        // shaderpack doesn't provide one then it can't be loaded
    }

    void load_pipeline_files(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, std::vector<pipeline_data>& output) {
        std::vector<fs::path> potential_pipeline_files;
        try {
            potential_pipeline_files = folder_access->get_all_items_in_folder("materials");
        } catch (filesystem_exception &exception) {
            NOVA_LOG(ERROR) << "Materials folder does not exist: " << exception.what();
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.resize(potential_pipeline_files.size());
        uint32_t num_pipelines = 0;
        ftl::AtomicCounter pipeline_load_tasks_remaining(task_scheduler);

        for(const fs::path& potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                task_scheduler->AddTask(&pipeline_load_tasks_remaining, load_single_pipeline, folder_access, potential_file, num_pipelines, output);
                num_pipelines++;
            }
        }

        task_scheduler->WaitForCounter(&pipeline_load_tasks_remaining, 0);
    }

    void load_single_pipeline(ftl::TaskScheduler *task_scheduler, folder_accessor_base* folder_access, const fs::path& pipeline_path, uint32_t out_idx, std::vector<pipeline_data>& output) {
        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);
        try {
            auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
            output[out_idx] = json_pipeline.get<pipeline_data>();

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse pipeline file " << pipeline_path.string() << ": " << err.what();
        }
    }

    void load_material_files(ftl::TaskScheduler * task_scheduler, folder_accessor_base* folder_access, std::vector<material_data>& output) {
        std::vector<fs::path> potential_material_files;
        try {
            potential_material_files = folder_access->get_all_items_in_folder("materials");

        } catch (filesystem_exception &exception) {
            NOVA_LOG(ERROR) << "Materials folder does not exist: " << exception.what();
            return;
        }

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.resize(potential_material_files.size());
        ftl::AtomicCounter material_load_tasks_remaining(task_scheduler);

        uint32_t num_materials = 0;

        for(const fs::path& potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                task_scheduler->AddTask(&material_load_tasks_remaining, load_single_material, folder_access, potential_file, num_materials, output);
                num_materials++;
            }
        }

        task_scheduler->WaitForCounter(&material_load_tasks_remaining, 0);\
    }

    void load_single_material(ftl::TaskScheduler * task_scheduler, folder_accessor_base* folder_access, const fs::path& material_path, uint32_t out_idx, std::vector<material_data>& output) {
        const auto material_bytes = folder_access->read_text_file(material_path);
        try {
            auto json_material = nlohmann::json::parse(material_bytes);
            auto material = json_material.get<material_data>();
            material.name = material_path.stem().string();
            output[out_idx] = material;

        } catch(nlohmann::json::parse_error& err) {
            NOVA_LOG(ERROR) << "Could not parse material file " << material_path.string() << ": " << err.what();
        }
    }
}
