#pragma once
#include <vector>

namespace nova::renderer {
#pragma region Forward declarations
    struct resource_t;
#pragma endregion

#pragma region Enums
    enum class resource_state {
        UNDEFINED,
        GENERAL,

        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY,
        DEPTH_STENCIL_READ_ONLY_ATTACHMENT,

        PRESENT_SOURCE,

        NON_FRAGMENT_SHADER_READ_ONLY,
        FRAGMENT_SHADER_READ_ONLY,

        TRANSFER_SOURCE,
        TRANSFER_DESTINATION,

    };
#pragma endregion

#pragma region Structs
    struct resource_barrier_t {
        /*!
         * \brief The type of barrier that you want
         * 
         * A RESOURCE_TRANSITION barrier can transition a resource from one state to another, while a SYNCHRONIZATION
         * barrier ensures that all reads or writes are complete before the next set of reads or writes can happen
         */
        enum class type {
            RESOURCE_TRANSITION,
            SYNCHRONIZATION,
        };

        type barrier_type;

        resource_t* resource_to_barrier;

        resource_state initial_state;
        resource_state final_state;
    };
#pragma endregion

#pragma region Command list API
    /*!
     * \brief An API-agnostic command list
     * 
     * Command lists are allocated from the render engine. Once allocated, ownership is passed to the callee. You can
     * then record whatever commands you want and submit the command list back to the render engine for execution on
     * the GPU. Once submitted, you may not record any more commands into the command list
     * 
     * There is one command list pool per swapchain image per thread. All the pools for one swapchain image are
     * reset at the beginning of a frame that renders to that swapchain image. This means that any command list
     * allocated in one frame will not be valid in the next frame. DO NOT hold on to command lists
     * 
     * Command lists are fully bound to ChaiScript
     */
    class command_list {
    public:
        enum class level {
            PRIMARY,
            SECONDARY,
        };

        command_list() = default;

        command_list(command_list&& old) noexcept = default;
        command_list& operator=(command_list&& old) noexcept = default;

        command_list(const command_list& other) = delete;
        command_list& operator=(const command_list& other) = delete;

        /*!
         * \brief Inserts a barrier so that all access to a resource before the barrier is resolved before any access
         * to the resource after the barrier
         * 
         * \param barriers All the resource barriers to use
         */
        virtual void resource_barrier(const std::vector<resource_barrier_t>& barriers) = 0;

        virtual void copy_buffer() = 0;

        virtual void execute_command_lists() = 0;

        virtual void begin_renderpass() = 0;
        virtual void end_renderpass() = 0;
        virtual void bind_pipeline() = 0;
        virtual void bind_material() = 0;

        virtual void bind_vertex_buffers() = 0;
        virtual void bind_index_buffers() = 0;
        virtual void draw_indexed() = 0;
        
        virtual ~command_list() = default;
    };
#prgama endregion
}
