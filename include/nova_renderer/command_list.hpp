/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#pragma once
#include <vector>
#include "render_engine.hpp"

namespace nova::renderer {
#pragma region Opqaue pointers
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

    enum resource_access_flags {
        INDEX_READ_BIT = 0x00000002,
        VERTEX_ATTRIBUTE_READ_BIT = 0x00000004,
        UNIFORM_READ_BIT = 0x00000008,
        INPUT_ATTACHMENT_READ_BIT = 0x00000010,
        SHADER_READ_BIT = 0x00000020,
        SHADER_WRITE_BIT = 0x00000040,
        COLOR_ATTACHMENT_READ_BIT = 0x00000080,
        COLOR_ATTACHMENT_WRITE_BIT = 0x00000100,
        DEPTH_STENCIL_ATTACHMENT_READ_BIT = 0x00000200,
        DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000400,
        TRANSFER_READ_BIT = 0x00000800,
        TRANSFER_WRITE_BIT = 0x00001000,
        HOST_READ_BIT = 0x00002000,
        HOST_WRITE_BIT = 0x00004000,
        MEMORY_READ_BIT = 0x00008000,
        MEMORY_WRITE_BIT = 0x00010000,
    };

    enum image_aspect_flags {
        COLOR = 0x00000001,
        DEPTH = 0x00000002,
        STENCIL = 0x00000004,
    };

    enum pipeline_stage_flags {
        TOP_OF_PIPE = 0x00000001,
        DRAW_INDIRECT = 0x00000002,
        VERTEX_INPUT = 0x00000004,
        VERTEX_SHADER = 0x00000008,
        TESSELLATION_CONTROL_SHADER = 0x00000010,
        TESSELLATION_EVALUATION_SHADER = 0x00000020,
        GEOMETRY_SHADER = 0x00000040,
        FRAGMENT_SHADER = 0x00000080,
        EARLY_FRAGMENT_TESTS = 0x00000100,
        LATE_FRAGMENT_TESTS = 0x00000200,
        COLOR_ATTACHMENT_OUTPUT = 0x00000400,
        COMPUTE_SHADER = 0x00000800,
        TRANSFER = 0x00001000,
        BOTTOM_OF_PIPE = 0x00002000,
        HOST = 0x00004000,
        ALL_GRAPHICS = 0x00008000,
        ALL_COMMANDS = 0x00010000,
        SHADING_RATE_IMAGE = 0x00400000,
        RAY_TRACING_SHADER = 0x00200000,
        ACCELERATION_STRUCTURE_BUILD = 0x02000000,
        TASK_SHADER = 0x00080000,
        MESH_SHADER = 0x00100000,
        FRAGMENT_DENSITY_PROCESS = 0x00800000,
    };
#pragma endregion

#pragma region Structs
    struct resource_barrier_t {
        resource_t* resource_to_barrier;

        resource_state initial_state;
        resource_state final_state;

        resource_access_flags access_before_barrier;
        resource_access_flags access_after_barrier;

        queue_type source_queue;
        queue_type destination_queue;

        union {
            struct {
                image_aspect_flags aspect;
            } image_memory_barrier;

            struct {
                uint64_t offset;
                uint64_t size;
            } buffer_memory_barrier;
        };
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
     * A command list may only be recorded to from one thread at a time
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
         * \param stages_before_barrier The pipeline stages that should be completed before the barriers take effect
         * \param stages_after_barrier The pipeline stages that must wait for the barrier
         * \param barriers All the resource barriers to use
         */
        virtual void resource_barriers(pipeline_stage_flags stages_before_barrier,
                                      pipeline_stage_flags stages_after_barrier,
                                      const std::vector<resource_barrier_t>& barriers) = 0;

        /*!
         * \brief Records a command to copy one region of a buffer to another buffer
         *
         * \param destination_buffer The buffer to write data to
         * \param destination_offset The offset in the destination buffer to start writing to. Measured in bytes
         * \param source_buffer The buffer to read data from
         * \param source_offset The offset in the source buffer to start reading from. Measures in bytes
         * \param num_bytes The number of bytes to copy
         *
         * \pre destination_buffer != nullptr
         * \pre destination_buffer is a buffer
         * \pre destination_offset is less than the size of destination_buffer
         * \pre source_buffer != nullptr
         * \pre source_buffer is a buffer
         * \pre source_offset is less than the size of source_buffer
         * \pre destination_offset plus num_bytes is less than the size of destination_buffer
         * \pre destination_offset plus num_bytes is less than the size of source_buffer
         */
        virtual void copy_buffer(resource_t* destination_buffer,
                                 uint64_t destination_offset,
                                 resource_t* source_buffer,
                                 uint64_t source_offset,
                                 uint64_t num_bytes) = 0;

        virtual void execute_command_lists(const std::vector<command_list*>& lists) = 0;

        virtual void begin_renderpass() = 0;
        virtual void end_renderpass() = 0;
        virtual void bind_pipeline() = 0;
        virtual void bind_material() = 0;

        virtual void bind_vertex_buffers() = 0;
        virtual void bind_index_buffer() = 0;
        virtual void draw_indexed_mesh() = 0;

        virtual ~command_list() = default;
    };
#pragma endregion
} // namespace nova::renderer
