#pragma once
#include "cached_buffer.hpp"

namespace nova::renderer {
    /*!
     * \brief A uniform buffer which holds a single struct
     *
     * \tparam DataType The type of the struct that this UBO holds
     */
    template <typename DataType>
    class struct_uniform_buffer : public cached_buffer {
    public:
        /*!
         * \brief Initializes this buffer
         *
         * `create_info.size`, the size of this buffer, is set to sizeof(DataType). This constructor doesn't make any
         * other modifications to the create_info (but cached_buffer's constructor does!)
         *
         * \param name The name of this buffer
         * \param device The device to create this buffer in
         * \param allocator The allocator to allocate this buffer's host and device memory from
         * \param create_info The create info for this buffer. Its size is set to sizeof(DataType) before it's sent to
         * cached_buffer's constructor
         * \param alignment The device's buffer alignment
         */
        struct_uniform_buffer(
            std::string name, VkDevice device, VmaAllocator allocator, VkBufferCreateInfo& create_info, uint64_t alignment)
            : cached_buffer(name,
                            device,
                            allocator,
                            {create_info.sType,
                             create_info.pNext,
                             create_info.flags,
                             sizeof(DataType),
                             create_info.usage,
                             create_info.sharingMode,
                             create_info.queueFamilyIndexCount,
                             create_info.pQueueFamilyIndices},
                            alignment) {}
    };
} // namespace nova::renderer
