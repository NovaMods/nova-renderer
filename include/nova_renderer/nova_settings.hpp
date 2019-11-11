#pragma once

#include <string>
#include <vector>

#include "nova_renderer/util/utils.hpp"

namespace nova::renderer {

    struct Semver {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;
    };

    enum class GraphicsApi {
        Vulkan,
        Dx12,
        Gl2,
    };

    class NovaSettingsAccessManager;

    /*!
     * \brief Anything which inherits from this class wants to know about the configuration and any changes to it
     */
    class ConfigListener {
    public:
        /*!
         * \brief Tells the listeners that there has been a change in the configuration
         *
         * This method is called throughout Nova's lifetime whenever a configuration value changes. This method should
         * handle changing configuration values such as the size of the window and what shaderpack the user has loaded
         *
         * Note that this method only receives the read-write config values (the 'settings' node)
         *
         * \param new_config The updated configuration
         */
        virtual void on_config_change(const NovaSettingsAccessManager& new_config) = 0;

        /*!
         * \brief Tells listeners that the configuration has been loaded
         *
         * When Nova starts up, this method is called on all config listeners, then on_config_change is called.
         * on_config_change should be used to listen for any config values that change throughout the program's life, so
         * then this method should be used for any initial configuration whose values will not change throughout the
         * program's lifetime. An example of this is reading in the bind points of the UBOs: the bind points won't change
         * throughout the program's life, so they should be handled in this function
         *
         * We may want to consider two config files: one for read-only values and one for read-write values. Probably a
         * good idea, but I don't feel like implementing that just yet
         *
         * \param config The configuration that was loaded
         */
        virtual void on_config_loaded(const NovaSettingsAccessManager& config) = 0;
    };

    struct NovaSettings {
        /*!
         * \brief Options for configuring the way mesh memory is allocated
         *
         * Nova tries to be clever and optimize how it draws meshes with indirect rendering. It shoves everything into
         * a handful of giant buffers, to facilitate indirect rendering. These options are how you configure that
         */
        struct BlockAllocatorSettings {
            /*!
             * \brief The total amount of memory that can be used
             *
             * This must be a whole-number multiple of `new_buffer_size`
             */
            uint32_t max_total_allocation = 1024 * 1024 * 1024;

            /*!
             * \brief The size of one buffer
             *
             * Nova doesn't allocate `max_total_allocation` memory initially. It only allocates a single buffer of
             * `new_buffer_size` size, then allocates new buffers as needed
             *
             * This number must be a whole-number multiple of `buffer_part_size`
             */
            uint32_t new_buffer_size = 16 * 1024 * 1024;

            /*!
             * \brief The size of one allocation from a buffer
             *
             * Nova gives meshes one or more allocations from a given buffer.
             */
            uint32_t buffer_part_size = 16 * 1024;
        };

        /*!
         * \brief All options to turn on debugging functionality
         */
        struct DebugOptions {
            /*!
             * \brief If false, all debugging behavior is disabled, even if individual options are turned on
             */
            bool enabled = false;

            /*!
             * \breif Controls if the API-specific validation layers are enabled
             *
             * This should be enabled most of the time for Nova developers and almost never for shaderpack authors.
             * Nova developers need it on to debug their Vulkan or DX12 usage, while Nova should be robust enough that
             * errors that the validation layers would catch never happen in a shipping build
             */
            bool enable_validation_layers = false;

            /*!
             * \brief Should Nova raise SIGINT when the validation layers detect an error?
             */
            bool break_on_validation_errors = true;

            /*!
             * \brief Enables GPU-based validation, which can check more situations then the normal debug layers but can cost a lot of
             * performance
             *
             * GPU-based validation checks for a number of errors like uninitialized descriptors, indexing a descriptor that doesn't exist,
             * or trying to access a resource that's in an incomplete state. These are great errors to check for, but checking for them
             * costs significant GPU time. Unless you're developing Nova, this should probably remain `false`
             */
            bool enable_gpu_based_validation = false;

            struct {
                /*!
                 * \brief If true, Nova will look for RenderDoc on your computer and will try to load it, letting you
                 * debug your shaderpack without leaving Nova
                 */
                bool enabled = false;

                /*!
                 * \brief The path to `renderdoc.dll` on your filesystem
                 */
                std::string renderdoc_dll_path = R"(C:\Program Files\RenderDoc\renderdoc.dll)";

                /*!
                 * \brief The base path for RenderDoc captures
                 */
                const char* capture_path = "logs/captures";

            } renderdoc;
        } debug;

        /*!
         * \brief Settings that Nova can change, but which are still stored in a config
         */
        struct CacheOptions {
            /*!
             * \brief The shaderpack that was most recently loaded
             *
             * Nova requires a shaderpack to render anything, so we need to know which one to load on application start
             */
            const char* loaded_shaderpack = "DefaultShaderpack";
        } cache;

        /*!
         * \brief Options about the window that Nova will live in
         */
        struct WindowOptions {
            /*!
             * \brief The title of the Window
             */
            const char* title = "Nova Renderer";

            /*!
             * \brief The width of the window
             */
            uint32_t width{};

            /*!
             * \brief The height of the window
             */
            uint32_t height{};
        } window;

        /*!
         * \brief Options that are specific to Nova's Vulkan rendering backend
         */
        struct VulkanOptions {
            /*!
             * \brief The application name to pass to Vulkan
             */
            const char* application_name = "Nova Renderer";

            /*!
             * \brief The application version to pass to Vulkan
             */
            Semver application_version = {0, 8, 4};
        } vulkan;

        /*!
         * \brief Options that are specific to Nova's DirectX 12 backend
         */
        struct Dx12Options {
        } dx12;

        /*!
         * \brief The rendering API to use
         *
         * DirectX 12 is only supported on Windows 10. On other platforms Vulkan will be used, regardless of what you've chosen
         */
        GraphicsApi api{};

        /*!
         * \brief Information about the system we're running on
         */
        struct SystemInfo {
            /*!
             * \brief Whether we're on a Unified Memory Architecture
             */
            bool is_uma = false;
        } system_info;

        uint32_t max_in_flight_frames = 3;

        /*!
         * \brief Settings for how Nova should allocate vertex memory
         */
        BlockAllocatorSettings vertex_memory_settings;

        /*!
         * \brief Settings for how Nova should allocate index memory
         */
        BlockAllocatorSettings index_memory_settings;
    };

    class NovaSettingsAccessManager { // Classes named Manager are an antipattern so yes
    public:
        NovaSettings settings;

        explicit NovaSettingsAccessManager(NovaSettings settings);

        /*!
         * \brief Registers the given iconfig_change_listener as an Observer
         */
        void register_change_listener(ConfigListener* new_listener);

        /*!
         * \brief Updates all the change listeners with the current state of the settings
         *
         * This method is public so that whatever changes values can delay calling it. You can set a bunch of options that
         * are pretty computationally intensive to change, the update listeners after all the values are changed
         *
         * Note that this method only send the read-write config values (children of the node 'settings') to the listeners
         */
        void update_config_changed();

        /*!
         * \brief Tells all the config listeners that the configuration has been loaded for the first time
         */
        void update_config_loaded();

        // Why did this take so long omg
        const NovaSettings* operator->() const;

    private:
        std::vector<ConfigListener*> config_change_listeners;
    };
} // namespace nova::renderer
