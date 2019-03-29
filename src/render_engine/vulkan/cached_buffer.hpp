/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include <string>
#include "../../util/vma_usage.hpp"

namespace nova::renderer {
    /*!
     * \brief A buffer which is stored both in both device memory and host memory
     *
     * The buffer exists in both host and device memory. You can get a pointer to the host buffer with `get_data<T>()`,
     * which will `reinterpret_cast` the host buffer to the template argument. Fill out the data as needed, then use
     * `record_upload_buffer` to record the buffer upload into a command list.
     *
     * Intended use case is buffers which require large amounts of data to be uploaded at once
     */
    class cached_buffer {
    public:
        cached_buffer() = default;

        /*!
         * \brief Best constructor ever
         *
         * \param name The name of the buffer
         * \param device The device to create the buffer on
         * \param allocator The VmaAllocator to allocate the host and device memory from
         * \param create_info The create info for your buffer. This is used for both the host buffer and the device
         * buffer. When creating the device buffer, the constructor add the usage flag
         * `VK_BUFFER_USAGE_TRANSFER_DST_BIT` to whatever flags the caller sets. When creating the host buffer, the
         * flags are set to only `VK_BUFFER_USAGE_TRANSFER_SRC_BIT`
         * \param alignment The device's buffer alignment
         */
        cached_buffer(
            std::string name, VkDevice device, VmaAllocator allocator, const VkBufferCreateInfo& create_info, VkDeviceSize alignment);

        cached_buffer(const cached_buffer& other) = delete;
        cached_buffer& operator=(const cached_buffer& other) = delete;

        cached_buffer(cached_buffer&& old) noexcept = default;
        cached_buffer& operator=(cached_buffer&& old) noexcept = default;

        virtual ~cached_buffer();

        /*!
         * \brief Provides access to the buffer in host memory
         *
         * \tparam UboStructType The type that you want to see the buffer as
         *
         * \return A pointer to the buffer in host memory
         */
        template <typename UboStructType>
        [[nodiscard]] UboStructType* get_data() {
            return reinterpret_cast<UboStructType*>(cpu_alloc_info.pMappedData);
        }

        [[nodiscard]] VmaAllocation& get_allocation();

        [[nodiscard]] VmaAllocationInfo& get_allocation_info();

        [[nodiscard]] const std::string& get_name() const;

        [[nodiscard]] const VkBuffer& get_vk_buffer() const;

        [[nodiscard]] VkDeviceSize get_size() const;

        [[nodiscard]] VkFence get_dummy_fence() const;

        /*!
         * \brief Records a command to transfer the host buffer to the device buffer
         *
         * This method won't add any barriers, since it doesn't know about how the buffer is used. You must add (or not
         * add) those yourself
         *
         * \param cmds The command buffer to record the buffer transfer command into
         *
         * \pre `vkBeginCommandBuffer` has been called on the command buffer
         */
        void record_buffer_upload(VkCommandBuffer cmds);

    protected:
        std::string name;
        VkDeviceSize alignment = 64;

        VmaAllocator allocator = nullptr;

        VkDevice device = nullptr;
        VkBuffer buffer = nullptr;

        VmaAllocation allocation = nullptr;
        VmaAllocationInfo allocation_info = {};

        VkBuffer cpu_buffer = nullptr;
        VmaAllocation cpu_allocation = nullptr;
        VmaAllocationInfo cpu_alloc_info = {};

        VkFence dummy_fence = nullptr;
    };
} // namespace nova::renderer

#endif // RENDERER_GL_UNIFORM_BUFFER_H
