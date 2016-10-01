/*!
 * \author David
 * \date 13-May-16.
 */

#ifndef RENDERER_GUI_RENDERER_H
#define RENDERER_GUI_RENDERER_H

#include <memory>
#include <atomic>

#include <mutex>

#include "model/gl/gl_vertex_buffer.h"
#include "view/uniform_buffer_store.h"
#include "interfaces/ivertex_buffer.h"
#include "mc_interface/mc_gui_objects.h"
#include "model/texture_manager.h"
#include "model/shaders/uniform_buffer_definitions.h"


namespace nova {
    namespace view {
        /*!
         * \brief Defines a bunch of methods to render different GUI elements, including buttons, text, and images
         *
         * Something interesting to note - GUI layout is all calculated in pixels, then all the vertices are scaled down by the
         * view matrix.
         *
         * The GUI renderer works directly with the data received from Minecraft. There's no wrapper classes, no conversions to
         * a format the renderer can handle better. This is mostly done because I don't hae a good reason to do it another way.
         * The design of the GUI renderer is such that using the data from MC directly is simpler.
         *
         * What is the design of the GUI renderer, you ask? Well, let me tell you. The entire GUI is laid out in a single VBO.
         * I use atlases to have the textures for multiple GUI elements available. MC does this too, but I shove every
         * GUI-related texture into the same atlas. This includes the current font. Then, when the GUI receives a GUI screen, I
         * compare it to the previous GUI screen. If it's exactly the same, I don't re-build the GUI VBO. If it's the same
         * screen but a button is pressed or an item is highlighted or something, I only change the updated element(s). I only
         * completely rebuild the GUI geometry when the new gui screen is different from the current GUI screen. I do this so I
         * don't have to send a lot of information to the GPU. While it's true that the GUI geometry wil be relatively small and
         * probably not a bottleneck, I want to ensure that every part of this mod is built for speed and efficiency.
         *
         * The GUI isn't rendered here, rather the render thread gets the GUI VBO from here and then parties.
         */
        class gui_geometry_builder {
        public:
            gui_geometry_builder(nova::model::texture_manager &textures,
                                 nova::model::uniform_buffer_store &uniform_buffers);

            ~gui_geometry_builder();

            /*!
             * \brief Sets the GUI screen to render as the given screen
             */
            void set_current_screen(mc_gui_screen *screen);

            model::gl_vertex_buffer& get_vertex_buffer();

            /*!
             * \brief Checks to see if a new GUI screen is available. If so, sets the current GUI screen to the new GUI screen
             * and re-builds GUI geometry
             */
            void update();

        private:
            std::vector<float> basic_unpressed_uvs = {
                    0.0f, 0.3359375f,
                    0.78125f, 0.3359375f,
                    0.0f, 0.4156963f,
                    0.78125f, 0.4156963f
            };

            std::vector<float> basic_pressed_uvs = {
                    0.0f, 0.2578112f,
                    0.78125, 0.2578112f,
                    0.0f, 0.3359375f,
                    0.78125f, 0.3359375f
            };

            std::vector<unsigned short> index_buffer = {
                    0, 1, 2,
                    2, 1, 3
            };

            mc_gui_screen cur_screen;

            nova::model::texture_manager &tex_manager;
            nova::model::uniform_buffer_store &ubo_manager;

            std::string WIDGETS_TEXTURE_NAME = "textures/gui/widgets.png";
            std::string GUI_SHADER_NAME = "gui";

            model::gl_vertex_buffer cur_screen_buffer;

            // Memory that will be accessed from both the render thread and the Java thread
            mc_gui_screen new_screen;
            std::mutex new_screen_guard;
            std::atomic<bool> has_screen_available;

            /*!
             * \brief Constructs the geometry needed to render the current GUI screen
             *
             * Note that the GUI screen does not include things like the spinning background on the main menu screen, because
             * that's going to be rendered as if it was a scene
             */
            void build_gui_geometry();

            void add_indices_for_button(std::vector<unsigned short> &indices, unsigned short start_pos);
        };

        /*!
         * \brief Compares two mc_gui_screen objects, determining if they represent the same visual data
         *
         * I'l like to have had this function in the same header file as the mc_gui_screen struct. However, I need
         * mc_gui_screen to be a C struct so I can properly assign to it from Java. The compiler yelled at me about "You
         * can't compare structs" so I couldn't use the == operator and here we are.
         */
        bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) const;

        /*!
         * \brief Determines whether or not the two given buttons are the same
         *
         * Two buttons are the same if they have the same position, size, and pressed status. If they can be drawn using
         * the exact same geometry and texture, then they are the same.
         *
         * \param button1 The first button to compare
         * \param button2 The second button to compare
         *
         * \return True if the buttons are the same, false otherwise
         */
        bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) const;

        bool compare_text(const char *text1, const char *text2) const;

        /*!
         * \brief Adds all the vertices from the given button to the given vertex buffer. uvs holds the uv
         * coordinates for this button
         *
         * \param vertex_buffer The vertex buffer to add vertices to
         * \param button The button to get vertices from
         * \param uvs The uv coordinates to use for this button
         */
        void add_vertices_from_button(std::vector<float> &vertex_buffer, const mc_gui_button &button, const std::vector<float> &uvs);

        /*!
         * \brief Adds the vertex with the given parameters to the given vertex buffer
         *
         * Note that the z position of the vertices is always set to 0. This is maybe what I want.
         *
         * \param vertex_buffer The thing to add vertices to
         * \param x The x position of the vertex
         * \param y The y position of the vertex
         * \param u The u texture coordiante of the vertex
         * \param v The v texture coordinate of the vertex
         */
        void add_vertex(std::vector<float> &vertex_buffer, int x, int y, float u, float v);
    }
}

#endif //RENDERER_GUI_RENDERER_H
