#pragma once

#include <nova_renderer/nova_renderer.hpp>
#include <nova_renderer/shaderpack_data.hpp>

namespace nova::renderer {

    class RenderGraph {
    public:
        /*!
         * \brief Adds a RenderPassCreateInfo to this render graph
         *
         *\param pass The render pass to insert into this render graph
         */
        void add_pass(shaderpack::RenderPassCreateInfo pass);

        /*!
         * \brief Compiles this render graph into a series of render passes
         *
         * This method optimizes the returned sequence of render passes as much as possible
         */
        [[nodiscard]] std::vector<Renderpass> compile();

    private:
        /*!
         * \brief Tells us if the compilation cache is dirty - aka if someone has added a renderpass since the last
         * time someone compiled this render graph
         */
        bool is_compile_cache_dirty = false;

        std::vector<shaderpack::RenderPassCreateInfo> passes;

        std::vector<Renderpass> compilation_result;
    };
} // namespace nova::renderer