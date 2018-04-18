#include "uniform_buffer.h"
#include "uniform_buffer.h"
#include "uniform_buffer.h"

namespace nova {

	uniform_buffer::uniform_buffer(std::string name, std::shared_ptr<render_context> context, vk::BufferCreateInfo create_info, uint64_t min_alloc_size, bool mapped) {
		VmaAllocationCreateInfo alloc_create = {};
		alloc_create.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		if(mapped) {
			alloc_create.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		}

		auto buffer_create_result = vmaCreateBuffer(context->allocator,
			reinterpret_cast<const VkBufferCreateInfo *>(&create_info), &alloc_create,
			reinterpret_cast<VkBuffer *>(&buffer), &allocation, &allocation_info);

		if(buffer_create_result != VK_SUCCESS) {
			LOG(ERROR) << "Could not allocate a an autobuffer because " << buffer_create_result;
		} else {
			LOG(TRACE) << "Auto buffer allocation success! Buffer ID: " << (long long)(VkBuffer)buffer;
		}
	}

	uniform_buffer::uniform_buffer(uniform_buffer && old) noexcept : name(old.name), min_alloc_size(old.min_alloc_size),
		context(old.context), device(old.device), buffer(old.buffer), allocation(old.allocation), allocation_info(old.allocation_info) {

		old.device = {};
		old.buffer = {};
		old.allocation = {};
		old.allocation_info = {};
	}

	uniform_buffer::~uniform_buffer() {
		if(buffer != vk::Buffer()) {
			LOG(TRACE) << "uniform_buffer: About to destroy buffer " << (long long)(VkBuffer)buffer;
			vmaDestroyBuffer(context->allocator, buffer, allocation);
		}
	}


	VmaAllocation &uniform_buffer::get_allocation() {
		return allocation;
	}

	VmaAllocationInfo &uniform_buffer::get_allocation_info() {
		return allocation_info;
	}
	const std::string& uniform_buffer::get_name() const {
		return name;
	}
}