#ifndef RENDERER_AUTO_ALLOCATED_BUFFER_H
#define RENDERER_AUTO_ALLOCATED_BUFFER_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "nova_renderer/util/result.hpp"
#include "cached_buffer.hpp"

namespace nova::renderer {
    /*!
     * \brief A chunk of a buffer
     *
     * This class can be added to a lit to mark the free areas in a buffer
     */
    struct auto_buffer_chunk {
        VkDeviceSize offset;
        VkDeviceSize range;
    };

    /*!
     * \brief A buffer that can be allocated from
     *
     * This buffer will attempt to automatically allocate space for you. It does not handle fragmentation at all since
     * the intended use case is a buffer with lots of things that are the same size
     *
     * A buffer allocated through this class is set up to move data from the CPU to the GPU. If that isn't the case in
     * the future then future DethRaid will have some work to do
     */
    class auto_buffer : public cached_buffer {
    public:
        auto_buffer() = default;

        /*!
         * \brief Creates a new auto-allocating buffer of the given size on the provided device
         * \param name The name of the new buffer
         * \param device The Vulkan device this buffer is tied to
         * \param allocator The allocator to allocate this buffer from
         * \param create_info Information about creating the buffer
         * \param alignment The minimum size of an allocation from this buffer
         */
        auto_buffer(const std::string& name, VkDevice device, VmaAllocator allocator, VkBufferCreateInfo& create_info, uint64_t alignment);

        auto_buffer(const auto_buffer& other) = delete;
        auto_buffer& operator=(const auto_buffer& other) = delete;

        auto_buffer(auto_buffer&& old) noexcept;
        auto_buffer& operator=(auto_buffer&& old) noexcept;

        ~auto_buffer() override = default;

        /*!
         * \brief Allocates a chunk of the underlying buffer for your personal user
         *
         * This method will give you the first free chunk it has, nothing else. If that leads to lots of fragmentation
         * then I'll deal with that later
         *
         * \param size The size, in bytes, of the space that you need
         * \return A representation of the allocation that's ready for use in a descriptor set
         */
        result<VkDescriptorBufferInfo> allocate_space(uint64_t size);

        /*!
         * \brief Frees the provided allocation
         *
         * \param to_free The allocation to free
         */
        void free_allocation(const VkDescriptorBufferInfo& to_free);

    private:
        std::vector<auto_buffer_chunk> chunks;
    };

    VkDeviceSize space_between(const auto_buffer_chunk& first, const auto_buffer_chunk& last);
} // namespace nova::renderer

#endif // RENDERER_AUTO_ALLOCATED_BUFFER_H
