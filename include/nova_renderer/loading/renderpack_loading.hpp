#pragma once

#include <vector>

#include "nova_renderer/filesystem/folder_accessor.hpp"
#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"

namespace nova::renderer::renderpack {
    /*!
     * \brief Loads all the data for a single renderpack
     *
     * This function reads the renderpack data from disk (either a folder od a zip file) and performs basic validation
     * to ensure both that the data is well-formatted JSON, but also to ensure that the data has all the fields that
     * Nova requires, e.g. a material must have at least one pass, a texture must have a width and a height, etc. All
     * generated warnings and errors are printed to the Nova logger
     *
     * If the renderpack can't be loaded, an empty optional is returned
     *
     * Note: This function is NOT thread-safe. It should only be called for a single thread at a time
     *
     * \param renderpack_name The name of the renderpack to loads
     * \return The renderpack, if it can be loaded, or an empty optional if it cannot
     */
    RenderpackData load_renderpack_data(const std::string& renderpack_name);

    std::vector<uint32_t> load_shader_file(const std::string& filename,
                                          filesystem::FolderAccessorBase* folder_access,
                                          rhi::ShaderStage stage,
                                          const std::vector<std::string>& defines = {});

    std::vector<uint32_t> compile_shader(const std::string& source,
                                        rhi::ShaderStage stage,
                                        rhi::ShaderLanguage source_language,
                                        filesystem::FolderAccessorBase* folder_accessor = nullptr);
} // namespace nova::renderer::renderpack
