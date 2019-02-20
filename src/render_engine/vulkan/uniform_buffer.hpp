/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include <vk_mem_alloc.h>
#include <string>

namespace nova {
    /*!
     * \brief A nice interface for uniform buffer objects
     */
    class uniform_buffer {
    public:
        uniform_buffer() = default;

        uniform_buffer(const std::string& name, VmaAllocator allocator, const VkBufferCreateInfo& create_info, uint64_t min_alloc_size, bool mapped);

        uniform_buffer(const uniform_buffer& other) = delete;
        uniform_buffer& operator=(const uniform_buffer& other) = delete;

        uniform_buffer(uniform_buffer&& old) noexcept;
        uniform_buffer& operator=(uniform_buffer&& old) noexcept;

        virtual ~uniform_buffer();

        VmaAllocation& get_allocation();
        VmaAllocationInfo& get_allocation_info();

        void set_data(const void* data, uint32_t size);

        const std::string& get_name() const;

        const VkBuffer& get_vk_buffer() const;

        uint64_t get_size() const;

    protected:
        std::string name;
        uint64_t min_alloc_size = 64;

        VmaAllocator allocator = nullptr;

        VkDevice device = nullptr;
        VkBuffer buffer = nullptr;

        VmaAllocation allocation = nullptr;
        VmaAllocationInfo allocation_info = {};
    };
} // namespace nova

#endif // RENDERER_GL_UNIFORM_BUFFER_H