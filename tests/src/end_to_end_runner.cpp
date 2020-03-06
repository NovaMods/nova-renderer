#include "general_test_setup.hpp"

#undef TEST

#include <iostream>

#ifdef __linux__

#include <csignal>

void sigsegv_handler(int signal);

void sigabrt_handler(int signal);

void at_exit_handler();

#include "../../src/util/linux_utils.hpp"

#endif

#include "nova_renderer/window.hpp"

namespace nova::renderer {
    RX_LOG("EndToEndRunner", logger);

    int main() {
#ifdef __linux__
        atexit(at_exit_handler);
#endif

        rx::array<char[FILENAME_MAX]> buff;
        getcwd(buff.data(), FILENAME_MAX);
        logger(rx::log::level::k_info, "Running in %s", buff.data());
        logger(rx::log::level::k_info, "Predefined resources at: %s", CMAKE_DEFINED_RESOURCES_PREFIX);

        NovaSettings settings;
        settings.vulkan.application_name = "Nova Renderer test";
        settings.vulkan.application_version = {0, 9, 0};
        settings.debug.enabled = true;
        settings.debug.enable_validation_layers = true;
        settings.debug.break_on_validation_errors = true;
        settings.debug.renderdoc.enabled = true;
        settings.window.width = 640;
        settings.window.height = 480;

        nova::filesystem::VirtualFilesystem::get_instance()->add_resource_root(CMAKE_DEFINED_RESOURCES_PREFIX);

        auto* renderer = rx::memory::g_system_allocator->create<NovaRenderer>(settings);

        renderer->load_renderpack("shaderpacks/DefaultShaderpack");

        NovaWindow& window = renderer->get_window();

        const static rx::array CUBE_VERTICES{
            FullVertex{{-1, -1, -1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{-1, -1, 1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{-1, 1, -1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{-1, 1, 1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{1, -1, -1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{1, -1, 1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{1, 1, -1}, {}, {}, {}, {}, {}, {}},
            FullVertex{{1, 1, 1}, {}, {}, {}, {}, {}, {}},
        };

        const static rx::array CUBE_INDICES{0, 1, 3, 6, 0, 2, 5, 0, 4, 6, 4, 0, 0, 3, 2, 5, 1, 0,
                                            3, 1, 5, 7, 4, 6, 4, 7, 5, 7, 6, 2, 7, 2, 3, 7, 3, 5};

        const MeshData cube = {7,
                               static_cast<uint32_t>(CUBE_INDICES.size()),
                               CUBE_VERTICES.data(),
                               CUBE_VERTICES.size() * sizeof(FullVertex),
                               CUBE_INDICES.data(),
                               CUBE_INDICES.size() * sizeof(uint32_t)};

        const MeshId mesh_id = renderer->create_mesh(cube);

        // Render one frame to upload mesh data
        renderer->execute_frame();

        // StaticMeshRenderableUpdateData data = {};
        // data.mesh = mesh_id;
        // data.initial_position = glm::vec3(0, 0, -5);

        // ReSharper disable once CppDeclaratorNeverUsed
        // const auto _ = renderer->add_renderable_for_material(FullMaterialPassName{"gbuffers_terrain", "forward"}, data);

        while(!window.should_close()) {
            renderer->execute_frame();
            window.poll_input();
        }

        rx::memory::g_system_allocator->destroy<NovaRenderer>(renderer);

        return 0;
    }

    // This is for scoping purposes so that things used in main
    // don't get destructed after rex_fini has been called
    int rex_main() {
        init_rex();
        auto ret = main();
        rex_fini();
        return ret;
    }
} // namespace nova::renderer

int main() {
#ifdef NOVA_LINUX
    signal(SIGSEGV, sigsegv_handler);
    signal(SIGABRT, sigabrt_handler);
#endif
    // Don't use nova::renderer::main(), see
    // rex_main() for more info
    return nova::renderer::rex_main();
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

void at_exit_handler() {
    std::cout << "Exiting program, stacktrace is:" << std::endl;
    nova_backtrace();
}

#endif
