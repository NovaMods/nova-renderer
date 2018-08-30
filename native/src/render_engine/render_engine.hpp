//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_RENDER_ENGINE_HPP
#define NOVA_RENDERER_RENDER_ENGINE_HPP

#include "../settings/settings.hpp"

namespace nova {
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
         * \brief Initializes the engine, does **NOT** open any window
         * \param settings The settings passed to nova
         *
         * \attention Called by nova
         */
        virtual void init(settings settings) = 0;

        /*!
         * \brief The engine name, for example "vulkan-1.1"
         * \attention Can be called before init(nova::settings)
         *
         * \return the engine name
         */
        virtual std::string get_engine_name() = 0;
    };
}

#endif //NOVA_RENDERER_RENDER_ENGINE_HPP
