/*!
 * \author ddubois
 * \date 21-Aug-18.
 */

#include "shaderpack_loading.hpp"
#include "../folder_accessor.hpp"
#include "../loading_utils.hpp"
#include "../zip_folder_accessor.hpp"
#include "../regular_folder_accessor.hpp"
#include "../json_utils.hpp"
#include "json_interop.hpp"
#include "shaderpack_validator.hpp"
#include "render_graph_builder.hpp"
#include <glslang/Public/ShaderLang.h>
#include <StandAlone/ResourceLimits.h>
#include "SPIRV/GlslangToSpv.h"
#include "../../tasks/task_scheduler.hpp"
#include "../../tasks/condition_counter.hpp"

namespace nova {
    folder_accessor_base* get_shaderpack_accessor(const fs::path& shaderpack_name);

	shaderpack_resources_data load_dynamic_resources_file(folder_accessor_base *folder_access);
	std::vector<render_pass_data> load_passes_file(folder_accessor_base *folder_access);
    std::vector<pipeline_data> load_pipeline_files(ttl::task_scheduler& task_scheduler, folder_accessor_base *folder_access);
    pipeline_data load_single_pipeline(ttl::task_scheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &pipeline_path);
    std::vector<material_data> load_material_files(ttl::task_scheduler& task_scheduler, folder_accessor_base *folder_access);
    material_data load_single_material(ttl::task_scheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &material_path);

    std::vector<uint32_t> load_shader_file(const fs::path& filename, folder_accessor_base* folder_access, EShLanguage stage, const std::vector<std::string>& defines);

    bool loading_failed = false;

    std::optional<shaderpack_data> load_shaderpack_data(const fs::path &shaderpack_name, ttl::task_scheduler &task_scheduler) {
        loading_failed = false;
        folder_accessor_base *folder_access = get_shaderpack_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem
        
        shaderpack_data data = {};

		ttl::task_node<void, shaderpack_resources_data> load_dynamic_resources_task([&] { return load_dynamic_resources_file(folder_access); });
        load_dynamic_resources_task.on_success(ttl::task_node<shaderpack_resources_data, void>([&data](shaderpack_resources_data& last_task_output) { data.resources = last_task_output; }));

		ttl::task_node<void, std::vector<render_pass_data>> load_passes_task([&] { return load_passes_file(folder_access); });
		load_passes_task.on_success(ttl::task_node<std::vector<render_pass_data>, void>([&data](std::vector<render_pass_data>& last_task_output) { data.passes = last_task_output; }));
		
		ttl::task_node<void, std::vector<pipeline_data>> load_pipelines_task([&] { return load_pipeline_files(task_scheduler, folder_access); });
		load_pipelines_task.on_success(ttl::task_node<std::vector<pipeline_data>, void>([&data](std::vector<pipeline_data>& last_task_output) { data.pipelines = last_task_output; }));
				
		ttl::task_node<void, std::vector<material_data>> load_materials_task([&] { return load_material_files(task_scheduler, folder_access); });
		load_materials_task.on_success(ttl::task_node<std::vector<material_data>, void>([&data](std::vector<material_data>& last_task_output) { data.materials = last_task_output; }));

		task_scheduler.add_task_graph(std::move(load_dynamic_resources_task));

		NOVA_LOG(TRACE) << "Kicked off all shaderpack data loading tasks";

        delete folder_access;

        if(loading_failed) {
            return {};

        } else {
            return std::make_optional(data);
        }
    }

    folder_accessor_base *get_shaderpack_accessor(const fs::path &shaderpack_name) {
        folder_accessor_base *folder_access = nullptr;
        fs::path path_to_shaderpack = shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            folder_access = new zip_folder_accessor(path_to_shaderpack);

        } else if(fs::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            folder_access = new regular_folder_accessor(path_to_shaderpack);

        } else {
            throw resource_not_found_exception(shaderpack_name.string());
        }

        return folder_access;
    }

	shaderpack_resources_data load_dynamic_resources_file(folder_accessor_base *folder_access) {
        std::string resources_string = folder_access->read_text_file("resources.json");
        try {
            auto json_resources = nlohmann::json::parse(resources_string.c_str());
            const validation_report report = validate_shaderpack_resources_data(json_resources);
            print(report);
            if(!report.errors.empty()) {
                loading_failed = true;
                return {};
            }

            return json_resources.get<shaderpack_resources_data>();

        } catch(resource_not_found_exception &) {
            // No resources defined.. I guess they think they don't need any?
            NOVA_LOG(WARN) << "No resources file found for shaderpack at " << folder_access->get_root();
            loading_failed = true;

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's resources.json: " << err.what();
            loading_failed = true;

        } catch(validation_failure_exception &err) {
            NOVA_LOG(ERROR) << "Could not validate resources.json: " << err.what();
            loading_failed = true;
        }

		NOVA_LOG(TRACE) << "load_dynamic_resources_file finished";
    }

    std::vector<render_pass_data> load_passes_file(folder_accessor_base *folder_access) {
        const auto passes_bytes = folder_access->read_text_file("passes.json");
        try {
            auto json_passes = nlohmann::json::parse(passes_bytes);
            auto passes = json_passes.get<std::vector<render_pass_data>>();

            std::unordered_map<std::string, render_pass_data> passes_by_name;
            passes_by_name.reserve(passes.size());
            for(const auto &pass : passes) {
                passes_by_name[pass.name] = pass;
            }

            const auto ordered_pass_names = order_passes(passes_by_name);
            passes.clear();
            for(const auto& named_pass : ordered_pass_names) {
                passes.push_back(passes_by_name.at(named_pass));
            }

            return passes;

        } catch(nlohmann::json::parse_error &err) {
            NOVA_LOG(ERROR) << "Could not parse your shaderpack's passes.json: " << err.what();
            loading_failed = true;
        }

        // Don't check for a resources_not_found exception because a shaderpack _needs_ a passes.json and if the
        // shaderpack doesn't provide one then it can't be loaded, so we'll catch that exception later on

		NOVA_LOG(TRACE) << "load_passes_file finished";
    }

    std::vector<pipeline_data> load_pipeline_files(ttl::task_scheduler& task_scheduler, folder_accessor_base *folder_access) {
        std::vector<fs::path> potential_pipeline_files;
        try {
            potential_pipeline_files = folder_access->get_all_items_in_folder("materials");
        } catch(const filesystem_exception &exception) {
			throw pipeline_load_failed("Materials folder does not exist", exception);
        }

		std::vector<pipeline_data> output;

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.reserve(potential_pipeline_files.size());
		std::vector<std::future<pipeline_data>> future_pipelines(potential_pipeline_files.size());

        // TODO: Find a way to make an "and" task that can handle this situation
		uint32_t i = 0;
        for(const fs::path &potential_file : potential_pipeline_files) {
            if(potential_file.extension() == ".pipeline") {
                // Pipeline file!
                future_pipelines[i] = task_scheduler.add_task(load_single_pipeline, folder_access, potential_file);
				NOVA_LOG(TRACE) << "Kicked off task to load pipeline " << potential_file;
				i++;
            }
        }

		future_pipelines.resize(i);

        for(std::future<pipeline_data>& future_pipeline : future_pipelines) {
			future_pipeline.wait();
			pipeline_data pipeline = future_pipeline.get();
			NOVA_LOG(TRACE) << "Finished waiting for load of pipeline " << pipeline.name;
			output.push_back(pipeline);
        }

        output.shrink_to_fit();

		NOVA_LOG(TRACE) << "load_pipeline_files finished";

		return output;
    }

    pipeline_data load_single_pipeline(ttl::task_scheduler* task_scheduler, folder_accessor_base* folder_access, const fs::path& pipeline_path) {
		NOVA_LOG(TRACE) << "Task to load pipeline " << pipeline_path << " started";
        const auto pipeline_bytes = folder_access->read_text_file(pipeline_path);

        auto json_pipeline = nlohmann::json::parse(pipeline_bytes);
		NOVA_LOG(TRACE) << "Parsed JSON from disk for pipeline " << pipeline_path;
        const validation_report report = validate_graphics_pipeline(json_pipeline);
		NOVA_LOG(TRACE) << "Finished validating JSON for pipeline " << pipeline_path;
        print(report);
        if(!report.errors.empty()) {
            loading_failed = true;
			NOVA_LOG(TRACE) << "Loading pipeline file " << pipeline_path << " failed";
            return {};
        }

        pipeline_data new_pipeline = json_pipeline.get<pipeline_data>();
		NOVA_LOG(TRACE) << "Parsed JSON into pipeline_data for pipeline " << pipeline_path;
        new_pipeline.vertex_shader.source = load_shader_file(new_pipeline.vertex_shader.filename, folder_access,
            EShLangVertex, new_pipeline.defines);

        if(new_pipeline.geometry_shader) {
            (*new_pipeline.geometry_shader).source = load_shader_file((*new_pipeline.geometry_shader).filename, 
                folder_access, EShLangGeometry, new_pipeline.defines);
        }

        if(new_pipeline.tessellation_control_shader) {
            (*new_pipeline.tessellation_control_shader).source = load_shader_file(
                (*new_pipeline.tessellation_control_shader).filename, folder_access, EShLangTessControl, 
                new_pipeline.defines);
        }
        if(new_pipeline.tessellation_evaluation_shader) {
            (*new_pipeline.tessellation_evaluation_shader).source = load_shader_file(
                (*new_pipeline.tessellation_evaluation_shader).filename, folder_access, EShLangTessEvaluation, 
                new_pipeline.defines);
        }

        if(new_pipeline.fragment_shader) {
            (*new_pipeline.fragment_shader).source = load_shader_file((*new_pipeline.fragment_shader).filename, 
                folder_access, EShLangFragment, new_pipeline.defines);
        }

		NOVA_LOG(TRACE) << "Load of pipeline " << pipeline_path << " succeeded";

        return new_pipeline;
    }

    std::vector<uint32_t> load_shader_file(const fs::path& filename, folder_accessor_base* folder_access, const EShLanguage stage, const std::vector<std::string>& defines) {
        static std::unordered_map<EShLanguage, std::vector<fs::path>> extensions_by_shader_stage = {
            {EShLangVertex,  {
                ".vert.spirv",
                ".vsh.spirv",
                ".vertex.spirv",

                ".vert",
                ".vsh",

                ".vertex",

                ".vert.hlsl",
                ".vsh.hlsl",
                ".vertex.hlsl",
            }},
            { EShLangFragment, {
                ".frag.spirv",
                ".fsh.spirv",
                ".fragment.spirv",

                ".frag",
                ".fsh",

                ".fragment",

                ".frag.hlsl",
                ".fsh.hlsl",
                ".fragment.hlsl",
            }},
            { EShLangGeometry, {
                ".geom.spirv",
                ".geo.spirv",
                ".geometry.spirv",

                ".geom",
                ".geo",

                ".geometry",

                ".geom.hlsl",
                ".geo.hlsl",
                ".geometry.hlsl",
            }},
            { EShLangTessEvaluation, {
                ".tese.spirv",
                ".tse.spirv",
                ".tess_eval.spirv",

                ".tese",
                ".tse",

                ".tess_eval",

                ".tese.hlsl",
                ".tse.hlsl",
                ".tess_eval.hlsl",
            }},
            { EShLangTessControl, {
                ".tesc.spirv",
                ".tsc.spirv",
                ".tess_control.spirv",

                ".tesc",
                ".tsc",

                ".tess_control",

                ".tesc.hlsl",
                ".tsc.hlsl",
                ".tess_control.hlsl",
            }}
        };

        std::vector<fs::path> extensions_for_current_stage = extensions_by_shader_stage.at(stage);

        for(const fs::path& extension : extensions_for_current_stage) {
            fs::path full_filename = filename;
            full_filename.replace_extension(extension);

            if(!folder_access->does_resource_exist(full_filename)) {
                continue;
            }

            glslang::TShader shader(stage);

            // TODO: Figure out how to handle shader options
            // Maybe add them into the preamble?

            // Check the extension to know what kind of shader file the user has provided. SPIR-V files can be loaded 
            // as-is, but GLSL, GLSL ES, and HLSL files need to be transpiled to SPIR-V
            if(extension.string().find(".spirv") != std::string::npos) {
                // SPIR-V file!
                return folder_access->read_spirv_file(full_filename);

            } else if(extension.string().find(".hlsl") != std::string::npos) {
                shader.setEnvInput(glslang::EShSourceHlsl, stage, glslang::EShClientVulkan, 0);

            } else {
                // GLSL files have a lot of possible extensions, but SPIR-V and HLSL don't!
                shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 0);
            }

            const std::string shader_source = folder_access->read_text_file(full_filename);
            auto* shader_source_data = shader_source.data();
            shader.setStrings(&shader_source_data, 1);
            const bool shader_compiled = shader.parse(&glslang::DefaultTBuiltInResource, 450, ECoreProfile, false, false, EShMessages(EShMsgVulkanRules | EShMsgSpvRules));

            const char* info_log = shader.getInfoLog();
            if(std::strlen(info_log) > 0) {
                const char* info_debug_log = shader.getInfoDebugLog();
                NOVA_LOG(INFO) << full_filename.string() << " compilation messages:\n" << info_log << "\n" << info_debug_log;
            }

            if(!shader_compiled) {
                throw shader_compilation_failed(info_log);
            }

            glslang::TProgram program;
            program.addShader(&shader);
            const bool shader_linked = program.link(EShMsgDefault);
            if(!shader_linked) {
                const char* program_info_log = program.getInfoLog();
                const char* program_debug_info_log = program.getInfoDebugLog();
                NOVA_LOG(ERROR) << "Program failed to link: " << program_info_log << "\n" << program_debug_info_log;
            }

            std::vector<uint32_t> spirv;
            GlslangToSpv(*program.getIntermediate(stage), spirv);

            fs::path dump_filename = filename.filename();
            dump_filename.replace_extension(std::to_string(stage) + ".spirv.generated");
            write_to_file(spirv, dump_filename);

            return spirv;
        }

        throw resource_not_found_exception("Could not find shader " + filename.string());
    }

    std::vector<material_data> load_material_files(ttl::task_scheduler& task_scheduler, folder_accessor_base* folder_access) {
        std::vector<fs::path> potential_material_files;
        try {
            potential_material_files = folder_access->get_all_items_in_folder("materials");

        } catch(filesystem_exception &exception) {
			throw material_load_failed("Materials folder does not exist", exception);
        }

		std::vector<material_data> output;

        // The resize will make this vector about twice as big as it should be, but there won't be any reallocating
        // so I'm into it
        output.reserve(potential_material_files.size());
		std::vector<std::future<material_data>> future_materials(potential_material_files.size());

		uint32_t i = 0;
        for(const fs::path &potential_file : potential_material_files) {
            if(potential_file.extension() == ".mat") {
                future_materials[i] = task_scheduler.add_task(load_single_material, folder_access, potential_file);
				i++;
            }
        }
		future_materials.resize(i);

		for(auto& future_material : future_materials) {
			future_material.wait();
			material_data material = future_material.get();
			NOVA_LOG(TRACE) << "Finished waiting for material " << material.name;
			output.push_back(material);
		}

		output.shrink_to_fit();
		NOVA_LOG(TRACE) << "load_material_files finished";

		return output;
    }

    material_data load_single_material(ttl::task_scheduler *task_scheduler, folder_accessor_base *folder_access, const fs::path &material_path) {
        const std::string material_text = folder_access->read_text_file(material_path);

        auto json_material = nlohmann::json::parse(material_text);
        const auto report = validate_material(json_material);
        print(report);
        if(!report.errors.empty()) {
            // There were errors, this material can't be loaded
            loading_failed = true;
			NOVA_LOG(TRACE) << "Load of material " << material_path << " failed";
            return {};
        }

        auto material = json_material.get<material_data>();
        material.name = material_path.stem().string();
		NOVA_LOG(TRACE) << "Load of material " << material_path << " succeeded";
        return material;
    }
}  // namespace nova
