/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include <string>
#include <vk_mem_alloc.h>

namespace nova::renderer {
    /*!
     * \brief A nice interface for uniform buffer objects
     */
    class uniform_buffer {
    public:
        uniform_buffer() = default;

        uniform_buffer(
            const std::string& name, VmaAllocator allocator, const VkBufferCreateInfo& create_info, uint64_t alignment, bool mapped);

        uniform_buffer(const uniform_buffer& other) = delete;
        uniform_buffer& operator=(const uniform_buffer& other) = delete;

        uniform_buffer(uniform_buffer&& old) noexcept;
        uniform_buffer& operator=(uniform_buffer&& old) noexcept;

        virtual ~uniform_buffer();

        void set_data(const void* data, uint32_t size);

        [[nodiscard]] VmaAllocation& get_allocation();

        [[nodiscard]] VmaAllocationInfo& get_allocation_info();

        [[nodiscard]] const std::string& get_name() const;

        [[nodiscard]] const VkBuffer& get_vk_buffer() const;

        [[nodiscard]] uint64_t get_size() const;

    protected:
        std::string name;
        uint64_t alignment = 64;

        VmaAllocator allocator = nullptr;

        VkDevice device = nullptr;
        VkBuffer buffer = nullptr;

        VmaAllocation allocation = nullptr;
        VmaAllocationInfo allocation_info = {};
    };
} // namespace nova::renderer

#endif // RENDERER_GL_UNIFORM_BUFFER_H
