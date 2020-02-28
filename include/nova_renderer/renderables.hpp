#pragma once

#include <atomic>

#include <glm/glm.hpp>
#include <rx/core/string.h>
#include <rx/core/vector.h>

namespace nova::renderer {
    struct FullVertex {
        glm::vec3 position;          // 12 bytes
        glm::vec3 normal;            // 12 bytes
        glm::vec3 tangent;           // 12 bytes
        uint32_t main_uv;            // 4 bytes
        uint32_t secondary_uv;       // 4 bytes
        uint32_t virtual_texture_id; // 4 bytes
        glm::vec4 additional_stuff;  // 12 bytes
    };

    static_assert(sizeof(FullVertex) % 16 == 0, "full_vertex struct is not aligned to 16 bytes!");

    /*!
     * \brief All the data needed to make a single mesh
     *
     * Meshes all have the same data. Chunks need all the mesh data, and they're most of the world. Entities, GUI,
     * particles, etc will probably have FAR fewer vertices than chunks, meaning that there's not a huge savings by
     * making them use special vertex formats
     */
    struct MeshData {
        size_t num_vertex_attributes{};
        uint32_t num_indices{};

        /*!
         * \brief Pointer to the vertex data of this mesh
         */
        const void* vertex_data_ptr{};

        /*!
         * \brief Number of bytes of vertex data
         */
        size_t vertex_data_size{};

        /*!
         * \brief Pointer to the index data of this mesh
         */
        const void* index_data_ptr{};

        /*!
         * \brief Number of bytes of index data
         */
        size_t index_data_size{};
    };

    using MeshId = uint64_t;

    struct StaticMeshRenderableUpdateData {
        glm::vec3 position{};
        glm::vec3 rotation{};
        glm::vec3 scale{1};

        bool visible = true;
    };

    struct StaticMeshRenderableCreateInfo : StaticMeshRenderableUpdateData {
        bool is_static = true;

        MeshId mesh{};
    };

    using RenderableId = uint64_t;

    static std::atomic<RenderableId> next_renderable_id;

    struct RenderableMetadata {
        RenderableId id = 0;

        rx::vector<rx::string> passes{};
    };

    struct RenderCommand {
        RenderableId id{};

        bool is_visible = true;

        glm::mat4 model_matrix{1};
    };

    struct StaticMeshRenderCommand : RenderCommand {};

    StaticMeshRenderCommand make_render_command(const StaticMeshRenderableCreateInfo& data, RenderableId id);
} // namespace nova::renderer
