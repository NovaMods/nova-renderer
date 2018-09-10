//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_RENDER_ENGINE_HPP
#define NOVA_RENDERER_RENDER_ENGINE_HPP

#include <memory>
#include "../settings/nova_settings.hpp"
#include "../util/macros.hpp"
#include "window.hpp"

NOVA_EXCEPTION(nova, render_engine_initialization_exception)
NOVA_EXCEPTION(nova, render_engine_rendering_exception)

namespace nova {
    struct ifence;

    /*!
     * \brief Abstract class for render backends
     *
     * The constructor should not make any initialization
     * All functions must be called after init(nova::settings) has been called except
     *   explicitly marked in the documentation
     */
    class render_engine {
    public:
        /*!
         * \brief The engine name, for example "vulkan-1.1"
         * \attention Can be called before init(nova::settings)
         *
         * C++ doesn't support static virtual methods, but if you don't provide this method then Nova won't compile so
         * it's documented here
         *
         * \return the engine name
         *
         * static const std::string get_engine_name() const;
         */

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~render_engine() = default;

        /*!
         * \brief Initializes the window with the given size, and creates the swapchain for that window
         * \param width The width, in pixels, of the desired window
         * \param height The height, in pixels of the desired window
         */
        virtual void open_window(uint32_t width, uint32_t height) = 0;

        virtual std::shared_ptr<iwindow> get_window() const = 0;

        /*!
         * \brief Sets the frame graph that should be used
         *
         * Thi method will build all the API-specific resources that are needed to render with the provided frame graph
         *
         * \param frame_graph The frame graph
         */
        virtual void set_frame_graph() = 0;

        /*!
         * \brief Renders a frame like so well, you guys
         */
        virtual void render_frame() = 0;

    protected:
        /*!
         * \brief Initializes the engine, does **NOT** open any window
         * \param settings The settings passed to nova
         *
         * Intentionally does nothing. This constructor serves mostly to ensure that concrete render engines have a
         * constructor that takes in some settings
         *
         * \attention Called by nova
         */
        explicit render_engine(const nova_settings &settings) {};
    };
}

#endif //NOVA_RENDERER_RENDER_ENGINE_HPP
