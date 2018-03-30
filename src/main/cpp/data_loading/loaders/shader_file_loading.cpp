/*!
 * \brief Implements functions to load a shaderpack from an unzipped shaderpack
 *
 * \author ddubois 
 * \date 22-Feb-18.
 */

#include "shader_loading.h"
#include "../../utils/utils.h"

namespace nova {
    // TODO: Expand with Bedrock names
    std::vector<std::string> vertex_extensions = {
            ".vert.spirv",
            ".vsh.spirv",
            ".vertex.spirv",

            ".vert",
            ".vsh",

            ".vertex",

            ".vert.hlsl",
            ".vsh.hlsl",
            ".vertex.hlsl",
    };

    // TODO: Expand with Bedrock names
    std::vector<std::string> fragment_extensions = {
            ".frag.spirv",
            ".fsh.spirv",
            ".fragment.spirv",

            ".frag",
            ".fsh",

            ".fragment",

            ".frag.hlsl",
            ".fsh.hlsl",
            ".fragment.hlsl",
    };

    // TODO: Expand with Bedrock names
    std::vector<std::string> geometry_extensions = {
            ".geom.spirv",
            ".geo.spirv",
            ".geometry.spirv",

            ".geom",
            ".geo",

            ".geometry",

            ".geom.hlsl",
            ".geo.hlsl",
            ".geometry.hlsl",
    };

    // TODO: Expand with Bedrock names
    std::vector<std::string> tess_eval_extensions = {
            ".tese.spirv",
            ".tse.spirv",
            ".tess_eval.spirv",

            ".tese",
            ".tse",

            ".tess_eval",

            ".tese.hlsl",
            ".tse.hlsl",
            ".tess_eval.hlsl",
    };

    // TODO: Expand with Bedrock names
    std::vector<std::string> tess_control_extensions = {
            ".tesc.spirv",
            ".tsc.spirv",
            ".tess_control.spirv",

            ".tesc",
            ".tsc",

            ".tess_control",

            ".tesc.hlsl",
            ".tsc.hlsl",
            ".tess_control.hlsl",
    };

    // TODO: Fill this in
    std::vector<fs::path> bedrock_filenames = {};

    std::vector<fs::path> optifine_filenames = {
            fs::path("gbuffers_basic"),
            fs::path("gbuffers_textured"),
            fs::path("gbuffers_textured_lit"),
            fs::path("gbuffers_skybasic"),
            fs::path("gbuffers_skytextured"),
            fs::path("gbuffers_clouds"),
            fs::path("gbuffers_terrain"),
            fs::path("gbuffers_terrain_solid"),
            fs::path("gbuffers_terrain_cutout_mip"),
            fs::path("gbuffers_terrain_cutout"),
            fs::path("gbuffers_damagedblock"),
            fs::path("gbuffers_water"),
            fs::path("gbuffers_block"),
            fs::path("gbuffers_beaconbeam"),
            fs::path("gbuffers_item"),
            fs::path("gbuffers_entities"),
            fs::path("gbuffers_armor_glint"),
            fs::path("gbuffers_spidereyes"),
            fs::path("gbuffers_hand"),
            fs::path("gbuffers_weather"),
            fs::path("composite"),
            fs::path("composite1"),
            fs::path("composite2"),
            fs::path("composite3"),
            fs::path("composite4"),
            fs::path("composite5"),
            fs::path("composite6"),
            fs::path("composite7"),
            fs::path("final"),
            fs::path("shadow"),
            fs::path("shadow_solid"),
            fs::path("shadow_cutout"),
            fs::path("deferred"),
            fs::path("deferred1"),
            fs::path("deferred2"),
            fs::path("deferred3"),
            fs::path("deferred4"),
            fs::path("deferred5"),
            fs::path("deferred6"),
            fs::path("deferred7"),
            fs::path("gbuffers_hand_water"),
            fs::path("deferred_last"),
            fs::path("composite_last"),
    };

    bool contains_bedrock_files(std::vector<filesystem::path> &files);
    bool contains_optifine_files(std::vector<filesystem::path> &files);

    /*!
     * \brief Tries to load a single shader file from a folder
     *
     * Tries appending each string in extensions to the shader path. If one of the extensions is a real extension
     * of the file, returns the full text of the file. If the file cannot be found with any of the provided
     * extensions, then a not_found is thrown
     *
     * \param shader_path The path to the shader
     * \param extensions A list of extensions to try
     * \return The full source of the shader file
     */
    shader_file load_shader_file(const fs::path &shader_path, const std::vector<std::string> &extensions);

    /*!
     * \brief Loads the shader file from the provided istream
     *
     * \param stream The istream to load the shader file from
     * \param shader_path The path to the shader file (useful mostly for includes)
     * \return A list of shader_line objects
     */
    std::vector<shader_line> read_shader_stream(std::istream &stream, const fs::path &shader_path);

    /*!
     * \brief Loads a file that was requested through a #include statement
     *
     * This function will recursively include files. There's nothing to check for an infinite include loop, so try to
     * not have any
     *
     * \param shader_path The path to the shader that includes the file
     * \param line The line in the shader that contains the #include statement
     * \return The full source of the included file
     */
    std::vector<shader_line> load_included_file(const fs::path &shader_path, const std::string &line);

    /*!
     * \brief Determines the full file path of an included file
     *
     * \param shader_path The path to the current shader
     * \param included_file_name The name of the file to include
     * \return The path to the included file
     */
    fs::path get_included_file_path(const fs::path &shader_path, const fs::path &included_file_name);

    std::unordered_map<std::string, std::vector<pipeline>> load_pipelines_from_folder(const fs::path &shaderpack_path) {
        std::vector<pipeline> pipelines = read_pipeline_files(shaderpack_path);
        if(pipelines.empty()) {
            LOG(WARNING) << "No pipelines defined by shaderpack. Attempting to guess the intended shaderpack format";

            auto files = get_shader_names_in_folder(shaderpack_path / "shaders");

            if(contains_bedrock_files(files)) {
                pipelines = parse_pipelines_from_json(get_default_bedrock_passes());

            } else if(contains_optifine_files(files)) {
                pipelines = parse_pipelines_from_json(get_default_optifine_passes());

            } else {
                LOG(FATAL) << "Cannot work with the format of this shaderpack. Please chose another one and try again";
            }
        } else {
            // TODO: Try to identify if the passes have at least one pass that matches either the default Bedrock
            // TODO: or Optifine setup
            // Right now I'm not dealing with that
        }

        LOG(INFO) << "Reading shaders from disk";
        auto sources = load_sources_from_folder(shaderpack_path / "shaders", pipelines);

        auto pipelines_by_pass = std::unordered_map<std::string, std::vector<pipeline>>{};

        for(auto& pipeline : pipelines) {
            pipeline.shader_sources = sources[pipeline.name];
            pipelines_by_pass[pipeline.pass.value()].push_back(pipeline);
        }

        return pipelines_by_pass;
    }

    std::vector<pipeline> read_pipeline_files(const fs::path& shaderpack_path) {
        // Nova calls them pipelines, but Bedrock calls them materials. To maintain compatibility Nova loads pipelines
        // from the `materials` folder. Yeah it's kinda gross... but tough
        auto pipelines_path = shaderpack_path / "materials";
        if(pipelines_path.empty()) {
            LOG(WARNING) << "No pipelines found at path " << pipelines_path << ", returning empty";
            return {};
        }
        auto pipelines = std::vector<pipeline>{};
        auto pipelines_itr = fs::directory_iterator(pipelines_path);

        for(const auto &item : pipelines_itr) {
            LOG(TRACE) << "Examing file " << item.path();
            if(item.path().extension() != ".material") {
                LOG(WARNING) << "Skipping non-pipeline file " << item.path();
                continue;
            }
            // I do like using temporary variables for everything...
            std::stringstream ss;
            ss << item.path();
            // std::path's stream insertion operator adds double quotes. yay. I'm so glad the std authors made
            // filesystem so straightforward to use
            auto stringpath = ss.str().substr(1);
            stringpath = stringpath.substr(0, stringpath.size() - 1);

            auto stream = std::ifstream(stringpath);
            auto pipelines_json = load_json_from_stream(stream);

            auto pipeline_definitions = parse_pipelines_from_json(pipelines_json);
            pipelines.insert(pipelines.end(), pipeline_definitions.begin(), pipeline_definitions.end());
        }

        return pipelines;
    }

    std::vector<fs::path> get_shader_names_in_folder(const fs::path& shaderpack_path) {
        auto filenames = std::vector<fs::path>{};

        auto files_itr = fs::directory_iterator(shaderpack_path);
        for(const auto& file : files_itr) {
            if(fs::is_regular_file(file.path())) {
                filenames.push_back(file.path().filename());
            }
        }

        return filenames;
    }

    std::unordered_map<std::string, shader_definition> load_sources_from_folder(const fs::path &shaders_path, const std::vector<pipeline> &pipelines) {
        std::unordered_map<std::string, shader_definition> sources;

        for(const auto& pipeline : pipelines) {
            auto shader_lines = shader_definition{};

            // The pass data is filled from parent passes, so we should have the fragment shader and vertex shader at
            // least
            if(!pipeline.vertex_shader) {
                LOG(ERROR) << "No vertex shader set for pass " << pipeline.name << "! Make sure that this pipeline or one of its parents sets the vertex shader";
            } else {
                shader_lines.vertex_source = load_shader_file(shaders_path / pipeline.vertex_shader.value(), vertex_extensions);
            }

            if(!pipeline.fragment_shader) {
                LOG(ERROR) << "No fragment shader set for pass " << pipeline.name << "! Make sure that this pipeline or one of its parents sets the fragment shader";
            } else {
                shader_lines.fragment_source = load_shader_file(shaders_path / pipeline.fragment_shader.value(), fragment_extensions);
            }

            // Check if we have geometry or tessellation shaders
            if(pipeline.geometry_shader) {
                LOG(DEBUG) << "Trying to load geometry shader";
                shader_lines.geometry_source = load_shader_file(shaders_path / pipeline.geometry_shader.value(), geometry_extensions);
            }

            if(pipeline.tessellation_control_shader && !pipeline.tessellation_evaluation_shader) {
                LOG(WARNING) << "You've set a tessellation control shader but not an evaluation shader. You need both for this pipeline to perform tessellation, so Nova will not perform tessellation for this pipeline";

            } else if(pipeline.tessellation_evaluation_shader && !pipeline.tessellation_control_shader) {
                LOG(WARNING) << "You've set a tessellation evaluation shader but not a control shader. You need both for this pipeline to perform tessellation, so Nova will not perform tessellation for this pipeline";

            } else if(pipeline.tessellation_control_shader && pipeline.tessellation_evaluation_shader){
                LOG(INFO) << "Trying to load tessellation shaders";
                shader_lines.tessellation_evaluation_source = load_shader_file(shaders_path / pipeline.tessellation_evaluation_shader.value(), tess_eval_extensions);
                shader_lines.tessellation_control_source = load_shader_file(shaders_path / pipeline.tessellation_control_shader.value(), tess_control_extensions);
            }

            sources[pipeline.name] = shader_lines;
        }

        return sources;
    }

    shader_file load_shader_file(const fs::path &shader_path, const std::vector<std::string> &extensions) {
        for(auto &extension : extensions) {
            auto full_shader_path = shader_path;
            full_shader_path += extension;
            LOG(TRACE) << "Trying to load shader file " << full_shader_path;

            std::ifstream stream(full_shader_path.string(), std::ios::in);
            if(stream.good()) {
                LOG(INFO) << "Loading shader file " << full_shader_path;
                auto loaded_shader_file = shader_file{};
                loaded_shader_file.lines = read_shader_stream(stream, full_shader_path);
                loaded_shader_file.language = language_from_extension(extension);
                return loaded_shader_file;

            } else {
                LOG(WARNING) << "Could not read file " << full_shader_path;
            }
        }

        throw resource_not_found(shader_path.string());
    }

    std::vector<shader_line> read_shader_stream(std::istream &stream, const fs::path &shader_path) {
        std::vector<shader_line> file_source;
        std::string line;
        auto line_counter = 1;
        while(std::getline(stream, line, '\n')) {
            if(line.compare("#include") == 0) {
                auto included_file = load_included_file(shader_path, line);
                file_source.insert(file_source.end(), std::begin(included_file), std::end(included_file));

            } else {
                file_source.push_back({line_counter, shader_path, line});
            }

            line_counter++;
        }

        return file_source;
    }

    std::vector<shader_line> load_included_file(const fs::path &shader_path, const std::string &line) {
        auto included_file_name = get_filename_from_include(line);
        auto file_to_include = get_included_file_path(shader_path, included_file_name);
        LOG(TRACE) << "Dealing with included file " << file_to_include;

        try {
            return load_shader_file(file_to_include, {""});
        } catch(resource_not_found& e) {
            throw std::runtime_error("Could not load included file " + file_to_include.string());
        }
    }

    /*!
     * \brief Gets the name of the shaderpack from the file path
     *
     * \param file_path The file path to get the shaderpack name from
     * \return The name of the shaderpack
     */
    auto get_shaderpack_name(const fs::path &file_path) {
        bool should_return_next = false;
        for(const auto& path_part : file_path) {
            if(should_return_next) {
                return path_part;
            }

            should_return_next = true;
        }

        LOG(ERROR) << "Never should have come here!";
        LOG(ERROR) << "That's a horrible error message so here's another one: Nova should have gotten the name of the shaderpack from a file path but it didn't so now we're here and there's no reasonable defaults and everything's aweful";
        return fs::path{};
    }

    fs::path get_included_file_path(const fs::path &shader_path, const fs::path &included_file_name) {
        if(included_file_name.is_absolute()) {
            // This is an absolute include and it should be relative to the root directory
            auto shaderpack_name = get_shaderpack_name(shader_path);
            return fs::path{"shaderpacks"} / shaderpack_name / "shaders" / included_file_name;

        } else {
            // The include file is a relative include, this one's actually simpler
            return shader_path.parent_path() / included_file_name;
        }
    }

    bool contains_bedrock_files(std::vector<filesystem::path> &files) {
        for(const auto &bedrock_name : bedrock_filenames) {
            if(find(files.begin(), files.end(), bedrock_name) != files.end()) {
                return true;
            }
        }
        return false;
    }

    bool contains_optifine_files(std::vector<filesystem::path> &files) {
        for(const auto& bedrock_name : bedrock_filenames) {
            if(find(files.begin(), files.end(), bedrock_name) != files.end()) {
                return true;
            }
        }
        return false;
    }

    std::unordered_map<std::string, render_pass> load_passes_from_folder(const fs::path& shaderpack_path) {
        auto passes_path = shaderpack_path / "passes.json";
        auto ss = std::stringstream{};
        ss << passes_path;
        auto stringpath = ss.str().substr(1);
        stringpath = stringpath.substr(0, stringpath.size() - 1);
        auto passes_stream = std::ifstream{stringpath};
        auto passes_json = load_json_from_stream(passes_stream);

        return parse_passes_from_json(passes_json);
    }

    std::unordered_map<std::string, texture_resource> load_texture_definitions_from_folder(const fs::path& shaderpack_path) {
        fs::path resources_path = shaderpack_path / "resources.json";
        if(!fs::exists(resources_path)) {
            // No resources - so let's just early out
            return {};
        }

        auto ss = std::stringstream{};
        ss << resources_path;
        auto stringpath = ss.str().substr(1);
        stringpath = stringpath.substr(0, stringpath.size() - 1);
        auto resoruces_stream = std::ifstream{stringpath};
        auto resoruces_json = load_json_from_stream(resoruces_stream);

        return parse_textures_from_json(resoruces_json["textures"]);
    }
}
