#pragma once
#include <vulkan/vulkan.hpp>

namespace nova::renderer {
    /*!
     * \brief Abstraction over a rendering device that implements Vulkan
     *
     * Handles things like GPU/CPU synchronization, memory management, and submitting commands to the actual device
     *
     * Note that, although the device may be working on commands for multiple frames worth of work, this abstraction
     * can only be used to record commands for a single frame at a time
     */
    class VulkanBackend {
    public:
        explicit VulkanBackend();

        /*!
         * \brief Waits for the GPU to finish all in-flight frames, then destroys all resources and generally cleans up
         */
        ~VulkanBackend();

        /*!
         * \brief Advances the internal frame index, waits for the previous frame of that index to complete, frees
         * resources that are scheduled for destruction, all that jazz
         */
        void begin_frame();

        /*!
         * \brief Retrieves a command buffer that may be used for the current frame
         */
        [[nodiscard]] vk::CommandBuffer get_command_buffer();

        /*!
         * \brief Batches the provided command list for execution when the current frame ends
         */
        void submit_command_buffer(vk::CommandBuffer buffer);

        /*!
         * \brief Submits all the batched command lists, then flips the swapchain
         */
        void end_frame();

        [[nodiscard]] vk::Instance get_instance() const;

        [[nodiscard]] vk::Device get_device() const;

    private:
        /*!
         * \brief Maximum number of frames we can submit to the GPU before waiting for any
         */
        const static uint32_t num_gpu_frames{3};

        vk::Instance instance;

        vk::Device device;

        vk::Queue graphics_queue;

        /*!
         *\brief Semaphores used to synchronize the GPU frames
         */
        std::array<vk::Semaphore, num_gpu_frames> frame_fences;

        /*!
         * \brief Command buffers to submit at the end of the current frame
         */
        std::vector<vk::CommandBuffer> batched_command_bufers;

        uint32_t frame_idx{0};
    };
} // namespace nova::renderer
