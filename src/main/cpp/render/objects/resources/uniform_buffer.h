/*!
 * \author David
 * \date 22-May-16.
 */

#ifndef RENDERER_GL_UNIFORM_BUFFER_H
#define RENDERER_GL_UNIFORM_BUFFER_H

#include "../../vulkan/render_context.h"

namespace nova {
    /*!
     * \brief A nice interface for uniform buffer objects
     */
    class uniform_buffer {
    public:
		uniform_buffer() = default;

        uniform_buffer(std::string name, std::shared_ptr<render_context> context, vk::BufferCreateInfo create_info, uint64_t min_alloc_size, bool mapped);

        uniform_buffer(uniform_buffer &&old) noexcept;

        virtual ~uniform_buffer();

        VmaAllocation& get_allocation();
        VmaAllocationInfo& get_allocation_info();

		const std::string& get_name() const;

		const vk::Buffer& get_vk_buffer() const;

		const uint64_t get_size() const;

    protected:
		std::string name;
		uint64_t min_alloc_size;

		std::shared_ptr<render_context> context;

		vk::Device device;
		vk::Buffer buffer;

		VmaAllocation allocation;
		VmaAllocationInfo allocation_info;
    };
}

#endif //RENDERER_GL_UNIFORM_BUFFER_H
