#pragma once 

// include vulkan API 
#include "radx_core.hpp"
#include "radx_device.hpp"
#include "../rhi_types.hpp"

namespace radx {
    
    class VmaAllocatedBuffer: public nova::renderer::rhi::Buffer, public std::enable_shared_from_this<VmaAllocatedBuffer>  {
    public:
        VmaAllocatedBuffer();
        // TODO: memory allocation based constructor

        VmaAllocatedBuffer(
            const std::shared_ptr<radx::Device>& device, 
            vk::DeviceSize dsize = sizeof(uint32_t), 
            vk::BufferUsageFlags bufferUsage = vk::BufferUsageFlagBits::eStorageBuffer, 
            VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY, bool alwaysMapped = false
        );

        // TODO: correct destructor for Nova Renderer
        //~VmaAllocatedBuffer() {
        //    vmaDestroyBuffer(*device, *this, *allocation);
        //};

        // Get mapped memory
        void* map();

        // GPU unmap memory
        void unmap();

        // vk::Device caster
        //operator vk::Buffer&() { return buffer; };
        operator const vk::Buffer&() const { return buffer; };
        operator const VkBuffer&() const { return (VkBuffer&)buffer; };

        // Allocation
        //operator VmaAllocation&() { return allocation; };
        operator const VmaAllocation&() const { return *allocation; };

        // AllocationInfo
        //operator VmaAllocationInfo&() { return allocationInfo; };
        operator const VmaAllocationInfo&() const { return *allocation; };
        
        // 
        operator const vk::DescriptorBufferInfo&() const { return bufInfo; };

    protected:
        void * mappedData = nullptr;
        vk::Buffer buffer{};
        std::shared_ptr<nova::renderer::rhi::VulkanMemoryAllocation> allocation{};
        std::shared_ptr<radx::Device> device{};
        vk::DescriptorBufferInfo bufInfo{};
    };

	class VmaAllocatedImage : public nova::renderer::rhi::Image, public std::enable_shared_from_this<VmaAllocatedImage> {
	public:
		void* mappedData = nullptr;
        vk::Image image{};
		vk::ImageView imageView{};
		vk::ImageLayout layout = vk::ImageLayout::eGeneral;
		vk::ImageSubresourceRange  srange{};
		vk::ImageSubresourceLayers slayers{};
		std::shared_ptr<nova::renderer::rhi::VulkanMemoryAllocation> allocation{};
		std::shared_ptr<radx::Device> device{};
		vk::DescriptorImageInfo imageDesc{};

	public:
		VmaAllocatedImage();
		VmaAllocatedImage(
			const std::shared_ptr<radx::Device>& device,
			vk::ImageViewType imageViewType = vk::ImageViewType::e2D,
			vk::Format format = vk::Format::eR8G8B8A8Unorm,
			vk::Extent2D dsize = { 2,2 },
			vk::ImageUsageFlags bufferUsage = vk::ImageUsageFlagBits::eStorage,
			VmaMemoryUsage vmaUsage = VMA_MEMORY_USAGE_GPU_ONLY, bool alwaysMapped = false
		);

		~VmaAllocatedImage() {
			//vmaDestroyImage(*device, *this, allocation);
		};

		// Get mapped memory
		void* map();

		// GPU unmap memory
		void unmap();

		// vk::Device caster
		//operator vk::Buffer&() { return buffer; };
		operator const vk::Image& () const { return image; };
		operator const VkImage& () const { return (VkImage&)image; };

		// Allocation
		//operator VmaAllocation&() { return allocation; };
		operator const VmaAllocation& () const { return *allocation; };

		// AllocationInfo
		//operator VmaAllocationInfo&() { return allocationInfo; };
		operator const VmaAllocationInfo& () const { return *allocation; };

		// 
		operator const vk::ImageView& () const { return imageView; };
        operator const vk::DescriptorImageInfo() const { return imageDesc; };
	};





    // TODO: buffer copying data and zero-initializer
    template<class T>
    class BufferRegion {
    public:
        BufferRegion(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) : buffer(buffer) {
            bufInfo.buffer = (vk::Buffer)*buffer;
            bufInfo.offset = offset;
            bufInfo.range = size * sizeof(T);
            //this->map();
        };

        T* const& map() { mapped = (T*)((uint8_t*)buffer->map() + bufInfo.offset); return mapped; };
        void unmap() { buffer->unmap(); };

        T* const& data() { this->map(); return mapped; };
        const T*& data() const { return mapped; };

        size_t size() const { return size_t(bufInfo.range / sizeof(T)); };
        const vk::DeviceSize& range() const { return bufInfo.range; };

        // at function 
        const T& at(const uintptr_t& i) const { return mapped[i]; };
        T& at(const uintptr_t& i) { return mapped[i]; };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { return at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T*& begin() const { return data(); };
        T* const& begin() { return map(); };

        // end ptr
        const T*& end() const { return &at(size() - 1ul); };
        T* end() { return &at(size() - 1ul); };

        operator const vk::DescriptorBufferInfo&() const { return bufInfo; };
        operator const vk::Buffer&() const { return *buffer; };
        const vk::DeviceSize& offset() const { return bufInfo.offset; };

    protected:
        T* mapped = {};
        std::shared_ptr<VmaAllocatedBuffer> buffer = {};
        vk::DescriptorBufferInfo bufInfo = {};
    };

    template<class T>
    class Vector {
    public:
        Vector() {}
        Vector(const std::shared_ptr<VmaAllocatedBuffer>& buffer, vk::DeviceSize size = 0ull, vk::DeviceSize offset = 0u) {
            region = std::make_shared<BufferRegion<T>>(buffer, size, offset);
        };
        Vector(const std::shared_ptr<BufferRegion<T>>& region) : region(region) {};
        Vector(const Vector<T>& vector) : region(vector.region) {};

        // map through
        T* const& map() { return region->map(); };
        void unmap() { return region->unmap(); };

        T* const& data() { return region->data(); };
        const T*& data() const { return region->data(); };

        // sizing 
        size_t size() const { return region->size(); };
        const vk::DeviceSize& range() const { return region->range(); };

        // at function 
        const T& at(const uintptr_t& i) const { return region->at(i); };
        T& at(const uintptr_t& i) { return region->at(i); };

        // array operator 
        const T& operator [] (const uintptr_t& i) const { at(i); };
        T& operator [] (const uintptr_t& i) { return at(i); };

        // begin ptr
        const T*& begin() const { region->begin(); };
        T* const& begin() { return region->begin(); };

        // end ptr
        const T* end() const { return region->end(); };
        T* end() { return region->end(); };

        // 
        operator const vk::DescriptorBufferInfo&() const { return *region; };
        operator const vk::Buffer&() const { return *region; };
        const vk::DeviceSize& offset() const { return region->offset(); };

    protected:
        std::shared_ptr<BufferRegion<T>> region = {};
    };
};
