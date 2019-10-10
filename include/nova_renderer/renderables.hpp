#pragma once

#include <atomic>

#include <glm/glm.hpp>

namespace nova::renderer {
    struct NOVA_API FullVertex {
        glm::vec3 position;          // 12 bytes
        glm::vec3 normal;            // 12 bytes
        glm::vec3 tangent;           // 12 bytes
        glm::u16vec2 main_uv;        // 4 bytes
        glm::u8vec2 secondary_uv;    // 2 bytes
        uint32_t virtual_texture_id; // 4 bytes
        glm::vec4 additional_stuff;  // 16 bytes
    };

    static_assert(sizeof(FullVertex) % 16 == 0, "full_vertex struct is not aligned to 16 bytes!");

    /*!
     * \brief All the data needed to make a single mesh
     *
     * Meshes all have the same data. Chunks need all the mesh data, and they're most of the world. Entities, GUI,
     * particles, etc will probably have FAR fewer vertices than chunks, meaning that there's not a huge savings by
     * making them use special vertex formats
     */
    struct NOVA_API MeshData {
        std::vector<FullVertex> vertex_data;
        std::vector<uint32_t> indices;
    };

    using MeshId = uint64_t;

    struct NOVA_API StaticMeshRenderableUpdateData {
        MeshId mesh;
    };

    struct NOVA_API StaticMeshRenderableData : StaticMeshRenderableUpdateData {
        glm::vec3 initial_position = {};
        glm::vec3 initial_rotation = {};
        glm::vec3 initial_scale = glm::vec3(1);

        bool is_static = true;
    };

    using RenderableId = uint64_t;

    static std::atomic<RenderableId> next_renderable_id;

    struct NOVA_API RenderableMetadata {
        RenderableId id = 0;

        std::vector<std::string> passes;
    };

    struct NOVA_API RenderableBase {
        RenderableId id = 0;

        bool is_visible = true;

        glm::mat4 model_matrix = glm::mat4(1);
    };

    struct StaticMeshRenderCommand : RenderableBase {};
} // namespace nova::renderer
