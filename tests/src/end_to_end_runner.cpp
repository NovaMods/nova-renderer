/*!
 * \author ddubois
 * \date 30-Aug-18.
 */

#include "general_test_setup.hpp"
#undef TEST

#include <iostream>

#ifdef __linux__
#include <csignal>
void sigsegv_handler(int signal);
void sigabrt_handler(int signal);
#include "../../src/util/linux_utils.hpp"
#endif

namespace nova::renderer {
    int main() {
        // TODO: fil out this test when the RHI is stable
        TEST_SETUP_LOGGER();

        std::array<char, FILENAME_MAX> buff{};
        getcwd(buff.data(), FILENAME_MAX);
        NOVA_LOG(DEBUG) << "Running in " << buff.data() << std::flush;
        NOVA_LOG(DEBUG) << "Predefined resources at: " << CMAKE_DEFINED_RESOURCES_PREFIX;

        NovaSettings settings;
        settings.api = GraphicsApi::Vulkan;
        settings.vulkan.application_name = "Nova Renderer test";
        settings.vulkan.application_version = {0, 8, 0};
        settings.debug.enabled = true;
        settings.debug.enable_validation_layers = true;
        settings.debug.renderdoc.enabled = true;
        settings.window.width = 640;
        settings.window.height = 480;

        try {
            const auto renderer = NovaRenderer::initialize(settings);

            renderer->load_shaderpack(CMAKE_DEFINED_RESOURCES_PREFIX "shaderpacks/DefaultShaderpack");

            std::shared_ptr<Window> window = renderer->get_engine()->get_window();

            MeshData cube = {};
            cube.vertex_data = {
                FullVertex{{-1, -1, -1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{-1, -1, 1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{-1, 1, -1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{-1, 1, 1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{1, -1, -1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{1, -1, 1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{1, 1, -1}, {}, {}, {}, {}, {}, {}},
                FullVertex{{1, 1, 1}, {}, {}, {}, {}, {}, {}},
            };
            cube.indices = {0, 1, 3, 6, 0, 2, 5, 0, 4, 6, 4, 0, 0, 3, 2, 5, 1, 0, 3, 1, 5, 7, 4, 6, 4, 7, 5, 7, 6, 2, 7, 2, 3, 7, 3, 5};

            const MeshId mesh_id = renderer->create_mesh(cube);

            // Render one frame to upload mesh data
            renderer->execute_frame();
            window->on_frame_end();

            StaticMeshRenderableData data = {};
            data.mesh = mesh_id;
            data.initial_position = glm::vec3(0, 0, -5);

            renderer->add_renderable_for_material(FullMaterialPassName{"gbuffers_terrain", "forward"}, data);
            
            while(!window->should_close()) {
                renderer->execute_frame();
                window->on_frame_end();
            }

            NovaRenderer::deinitialize();

            return 0;
        }
        catch(const std::exception& e) {
            NOVA_LOG(ERROR) << e.what();
            return -1;
        }

        return 0;
    }
} // namespace nova::renderer

int main() {
#ifdef NOVA_LINUX
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigabrt_handler);
#endif
    return nova::renderer::main();
}

#ifdef __linux__
void sigsegv_handler(int sig) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    signal(sig, SIG_IGN);

    std::cerr << "!!!SIGSEGV!!!" << std::endl;
    nova_backtrace();

    _exit(1);
}

void sigabrt_handler(int sig) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    signal(sig, SIG_IGN);

    std::cerr << "!!!SIGABRT!!!" << std::endl;
    nova_backtrace();

    _exit(1);
}
#endif
