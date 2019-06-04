#pragma once 

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <future>
#include <fstream>

#include "radx_core.hpp"
#include "radx_internal.hpp"
#include "radx_shaders.hpp"
#include "radx_utils.hpp"
#include "radx_buffer.hpp"

namespace radx {

#ifdef RADX_IMPLEMENTATION

    // TODO: already allocated memory based constructor
    VmaAllocatedBuffer::VmaAllocatedBuffer(
        const std::shared_ptr<radx::Device>& device,
        vk::DeviceSize dsize,
        vk::BufferUsageFlags bufferUsage,
        VmaMemoryUsage vmaUsage, bool alwaysMapped
    ) : device(device) {

        // Create the buffer object without memory.
        vk::BufferCreateInfo ci{};
        ci.size = dsize;
        ci.usage = bufferUsage;
        ci.sharingMode = vk::SharingMode::eExclusive;
        ci.queueFamilyIndexCount = device->queueFamilyIndices().size();
        ci.pQueueFamilyIndices = device->queueFamilyIndices().data();

        // create memory allocation shared memory 
        this->allocation = std::make_shared<nova::renderer::rhi::VulkanMemoryAllocation>();

        // 
        VmaAllocationCreateInfo aci{};
        aci.flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
        aci.usage = this->allocation->usage = vmaUsage;

        //
        vmaCreateBuffer(*device, (VkBufferCreateInfo*)&ci, &aci, (VkBuffer*)&buffer, &allocation->allocation, &allocation->info);

        // 
        bufInfo = { buffer, 0, VK_WHOLE_SIZE };
    };

    // Get mapped memory
    void* VmaAllocatedBuffer::map() {
        if (this->allocation->usage == VMA_MEMORY_USAGE_GPU_ONLY && !allocation->info.pMappedData) {
            vmaMapMemory(*device, allocation->allocation, &mappedData);
        }
        else {
            mappedData = allocation->info.pMappedData;
        };
        return mappedData;
    };

    // GPU unmap memory
    void VmaAllocatedBuffer::unmap() {
        if (this->allocation->usage == VMA_MEMORY_USAGE_GPU_ONLY && mappedData) {
            vmaUnmapMemory(*device, allocation->allocation);
        };
    };


    // TODO: better image constructor 
	VmaAllocatedImage::VmaAllocatedImage(
		const std::shared_ptr<radx::Device>& device,
		vk::ImageViewType imageViewType, 
		vk::Format format,
		vk::Extent2D dsize,
		vk::ImageUsageFlags imageUsage,
		VmaMemoryUsage vmaUsage, 
		bool alwaysMapped
	) {
		// result will no fully handled
		VkResult result = VK_ERROR_INITIALIZATION_FAILED;

		// init image dimensional type
		vk::ImageType imageType = vk::ImageType::e2D; bool isCubemap = false;
		switch (vk::ImageViewType(imageViewType)) {
			case vk::ImageViewType::e1D: imageType = vk::ImageType::e1D; break;
			case vk::ImageViewType::e1DArray: imageType = vk::ImageType::e2D; break;
			case vk::ImageViewType::e2D: imageType = vk::ImageType::e2D; break;
			case vk::ImageViewType::e2DArray: imageType = vk::ImageType::e3D; break;
			case vk::ImageViewType::e3D: imageType = vk::ImageType::e3D; break;
			case vk::ImageViewType::eCube: imageType = vk::ImageType::e3D; isCubemap = true; break;
			case vk::ImageViewType::eCubeArray: imageType = vk::ImageType::e3D; isCubemap = true; break;
		};


		// additional usage
		//auto usage = vk::ImageUsageFlags(cinfo.usage) | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
		auto usage = VkImageUsageFlags(imageUsage) | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		// image memory descriptor
#ifdef VRT_ENABLE_VEZ_INTEROP
		auto imageInfo = VezImageCreateInfo{};
#else
		auto imageInfo = VkImageCreateInfo(vk::ImageCreateInfo{});
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VkSharingMode(vk::SharingMode::eExclusive);
#endif

		// unified create structure
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VkImageType(imageType);
		imageInfo.arrayLayers = 1; // unsupported
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.extent = VkExtent3D{ dsize.width, dsize.height, (isCubemap ? 6u : 1u) };
		imageInfo.format = VkFormat(format);
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.usage = usage;
		imageInfo.flags = {};

		imageInfo.queueFamilyIndexCount = 0;//device->_familyIndices.size();
		imageInfo.pQueueFamilyIndices = nullptr;//device->_familyIndices.data();


        this->allocation = std::make_shared<nova::renderer::rhi::VulkanMemoryAllocation>();

		// create image with allocation
#ifdef VRT_ENABLE_VEZ_INTEROP
		VezMemoryFlags mem = VEZ_MEMORY_GPU_ONLY;
		if (vezCreateImage(device->_device, mem, &imageInfo, &vtDeviceImage->_image) == VK_SUCCESS) { result = VK_SUCCESS; };
#else
		VmaAllocationCreateInfo allocCreateInfo = {};
		allocCreateInfo.usage = this->allocation->usage = vmaUsage;
		if (vmaCreateImage(*device, &imageInfo, &allocCreateInfo, (VkImage*)&image, &allocation->allocation, &allocation->info) == VK_SUCCESS) { result = VK_SUCCESS; };
#endif


		// subresource range
		srange.levelCount = 1;
		srange.layerCount = 1;
		srange.baseMipLevel = 0;
		srange.baseArrayLayer = 0;
		srange.aspectMask = vk::ImageAspectFlagBits::eColor;

		// subresource layers
		slayers.layerCount = srange.layerCount;
		slayers.baseArrayLayer = srange.baseArrayLayer;
		slayers.aspectMask = srange.aspectMask;
		slayers.mipLevel = srange.baseMipLevel;


		// image view for usage
#ifdef VRT_ENABLE_VEZ_INTEROP
		auto vinfo = VezImageViewCreateInfo{};
		vinfo.subresourceRange = *(VezImageSubresourceRange*)(&vtDeviceImage->_subresourceRange.baseMipLevel);
#else
		auto vinfo = VkImageViewCreateInfo(vk::ImageViewCreateInfo{});
		vinfo.subresourceRange = srange;
		vinfo.flags = {};
#endif
		vinfo.pNext = nullptr;
		vinfo.components = VkComponentMapping{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		vinfo.format = VkFormat(format);
		vinfo.image = image;
		vinfo.viewType = VkImageViewType(imageViewType);

#ifdef VRT_ENABLE_VEZ_INTEROP
		vezCreateImageView(device->_device, &vinfo, &vtDeviceImage->_imageView);
		vtDeviceImage->_initialLayout = vtDeviceImage->_layout;
#else
		imageView = vk::Device(*device).createImageView(vk::ImageViewCreateInfo(vinfo));
#endif

		// anyways create static descriptor
		imageDesc = { {}, imageView, vk::ImageLayout::eGeneral };
	};



	// Get mapped memory
	void* VmaAllocatedImage::map() {
		if (this->allocation->usage == VMA_MEMORY_USAGE_GPU_ONLY && !allocation->info.pMappedData) {
			vmaMapMemory(*device, allocation->allocation, &mappedData);
		}
		else {
			mappedData = allocation->info.pMappedData;
		};
		return mappedData;
	};

	// GPU unmap memory
	void VmaAllocatedImage::unmap() {
		if (this->allocation->usage == VMA_MEMORY_USAGE_GPU_ONLY && mappedData) {
			vmaUnmapMemory(*device, allocation->allocation);
		};
	};





    std::shared_ptr<Device> Device::initialize(const vk::Device& device, std::shared_ptr<radx::PhysicalDeviceHelper> physicalHelper) {
        this->physicalHelper = physicalHelper;
        this->device = device;

        // get VMA allocator for device
        if (this->physicalHelper->allocator)
        {
            this->allocator = this->physicalHelper->allocator;
        };


        if (!this->allocator)
        {
#ifdef VOLK_H_
            // load API calls for context
            volkLoadDevice(VkDevice(*this));

            // create VMA memory allocator (with Volk support)
            VolkDeviceTable vktable;
            volkLoadDeviceTable(&vktable, VkDevice(*this));

            // VMA functions with Volk compatibility
            VmaVulkanFunctions vfuncs = {};
            vfuncs.vkAllocateMemory = vktable.vkAllocateMemory;
            vfuncs.vkBindBufferMemory = vktable.vkBindBufferMemory;
            vfuncs.vkBindImageMemory = vktable.vkBindImageMemory;
            vfuncs.vkCreateBuffer = vktable.vkCreateBuffer;
            vfuncs.vkCreateImage = vktable.vkCreateImage;
            vfuncs.vkDestroyBuffer = vktable.vkDestroyBuffer;
            vfuncs.vkDestroyImage = vktable.vkDestroyImage;
            vfuncs.vkFreeMemory = vktable.vkFreeMemory;
            vfuncs.vkGetBufferMemoryRequirements = vktable.vkGetBufferMemoryRequirements;
            vfuncs.vkGetBufferMemoryRequirements2KHR = vktable.vkGetBufferMemoryRequirements2KHR;
            vfuncs.vkGetImageMemoryRequirements = vktable.vkGetImageMemoryRequirements;
            vfuncs.vkGetImageMemoryRequirements2KHR = vktable.vkGetImageMemoryRequirements2KHR;
            vfuncs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
            vfuncs.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
            vfuncs.vkMapMemory = vktable.vkMapMemory;
            vfuncs.vkUnmapMemory = vktable.vkUnmapMemory;
            vfuncs.vkInvalidateMappedMemoryRanges = vktable.vkInvalidateMappedMemoryRanges;
#endif

            // create Vma allocator
            VmaAllocatorCreateInfo allocatorInfo = {};
#ifdef VOLK_H_
            allocatorInfo.pVulkanFunctions = &vfuncs;
#endif
            allocatorInfo.physicalDevice = VkPhysicalDevice(*this->physicalHelper);
            allocatorInfo.device = VkDevice(*this);
            allocatorInfo.preferredLargeHeapBlockSize = 16 * sizeof(uint32_t);
            allocatorInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT || VMA_ALLOCATION_CREATE_MAPPED_BIT;
            allocatorInfo.pAllocationCallbacks = nullptr;
            allocatorInfo.pHeapSizeLimit = nullptr;
            vmaCreateAllocator(&allocatorInfo, &this->allocator);
        };

        // descriptor pool
        if (!this->descriptorPool)
        {
            // pool sizes, and create descriptor pool
            std::vector<vk::DescriptorPoolSize> psizes = { };
            psizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageBuffer).setDescriptorCount(128));
            psizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageTexelBuffer).setDescriptorCount(128));
            psizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eInlineUniformBlockEXT).setDescriptorCount(128));
            psizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(128));
            psizes.push_back(vk::DescriptorPoolSize().setType(vk::DescriptorType::eAccelerationStructureNV).setDescriptorCount(128));

            vk::DescriptorPoolInlineUniformBlockCreateInfoEXT inlineDescPool{};
            inlineDescPool.maxInlineUniformBlockBindings = 2;
            this->descriptorPool = vk::Device(*this).createDescriptorPool(vk::DescriptorPoolCreateInfo().setPNext(&inlineDescPool).setPPoolSizes(psizes.data()).setPoolSizeCount(psizes.size()).setMaxSets(256).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBindEXT));
        };

        // pipeline cache 
        if (!this->pipelineCache)
        {
            this->pipelineCache = vk::Device(*this).createPipelineCache(vk::PipelineCacheCreateInfo());
        };

        { // create descriptor layouts 
            const auto pbindings = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound | vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind | vk::DescriptorBindingFlagBitsEXT::eVariableDescriptorCount | vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;
            const auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(&pbindings);
            const auto vkpi = vk::DescriptorSetLayoutCreateInfo().setPNext(&vkfl);

            {
                const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                    vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 2, vk::ShaderStageFlagBits::eCompute), // keys cache
                    //vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values cache
                    vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // radice cache
                    vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // histogram of radices (every work group)
                    vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // prefix-sum of radices (every work group)
                    vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // keys references
                    vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eInlineUniformBlockEXT, sizeof(uint32_t), vk::ShaderStageFlagBits::eCompute), // inline uniform data of algorithms
                    //vk::DescriptorSetLayoutBinding(7, vk::DescriptorType::eStorageTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                    //vk::DescriptorSetLayoutBinding(8, vk::DescriptorType::eUniformTexelBuffer, 1, vk::ShaderStageFlagBits::eCompute),
                };

                const std::vector<vk::DescriptorBindingFlagsEXT> _bindingFlags = { vk::DescriptorBindingFlagBitsEXT::ePartiallyBound, {}, {}, {}, {}, {} };
                const auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(_bindingFlags.data()).setBindingCount(_bindingFlags.size());
                descriptorLayouts.push_back(device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPNext(&vkfl).setPBindings(_bindings.data()).setBindingCount(_bindings.size())));
            };

            {
                const std::vector<vk::DescriptorSetLayoutBinding> _bindings = {
                    vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 2, vk::ShaderStageFlagBits::eCompute), // keys in
                    //vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // values in
                    vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eInlineUniformBlockEXT, sizeof(uint32_t), vk::ShaderStageFlagBits::eCompute)
                };
                const std::vector<vk::DescriptorBindingFlagsEXT> _bindingFlags = { vk::DescriptorBindingFlagBitsEXT::ePartiallyBound, vk::DescriptorBindingFlagBitsEXT::ePartiallyBound };
                const auto vkfl = vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT().setPBindingFlags(_bindingFlags.data()).setBindingCount(_bindingFlags.size());
                descriptorLayouts.push_back(device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(vkpi).setPNext(&vkfl).setPBindings(_bindings.data()).setBindingCount(_bindings.size())));
            };
        };

        return shared_from_this();
    };



    InternalInterface& InternalInterface::buildDescriptorSet() {
        std::vector<vk::DescriptorSetLayout> dsLayouts = { device->getDescriptorSetLayoutSupport().at(0) };
        this->descriptorSet = vk::Device(*device).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(*device).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1)).at(0);




        // if no has buffer, set it!
        if (!this->countMaximBufferInfo.buffer) this->countMaximBufferInfo.buffer = (vk::Buffer)*bufferMemory;
        if (!this->keysStoreBufferInfo.buffer) this->keysStoreBufferInfo.buffer = (vk::Buffer)*bufferMemory;
        if (!this->keysBackupBufferInfo.buffer) this->keysBackupBufferInfo.buffer = (vk::Buffer)*bufferMemory;
        if (!this->keysCacheBufferInfo.buffer) this->keysCacheBufferInfo.buffer = (vk::Buffer)*bufferMemory;
        if (!this->histogramBufferInfo.buffer) this->histogramBufferInfo.buffer = (vk::Buffer)*bufferMemory;
        if (!this->prefixScansBufferInfo.buffer) this->prefixScansBufferInfo.buffer = (vk::Buffer)*bufferMemory;

        // write into descriptor set
        const auto writeTmpl = vk::WriteDescriptorSet(this->descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo(&this->keysStoreBufferInfo),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo(&this->keysBackupBufferInfo).setDstArrayElement(1),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(2).setPBufferInfo(&this->keysCacheBufferInfo),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(3).setPBufferInfo(&this->histogramBufferInfo),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(4).setPBufferInfo(&this->prefixScansBufferInfo),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(5).setPBufferInfo(&this->countMaximBufferInfo),

            //vk::WriteDescriptorSet(writeTmpl).setDstBinding(7).setDescriptorType(vk::DescriptorType::eStorageTexelBuffer).setPTexelBufferView(&this->keyExtraBufferViewStore),
            //vk::WriteDescriptorSet(writeTmpl).setDstBinding(8).setDescriptorType(vk::DescriptorType::eUniformTexelBuffer).setPTexelBufferView(&this->keyExtraBufferViewU8x4),
        };

        // inline descriptor 
        vk::WriteDescriptorSetInlineUniformBlockEXT inlineDescriptorData{};
        {
            inlineDescriptorData.dataSize = sizeof(uint32_t);
            inlineDescriptorData.pData = &this->maxElementCount;
            writes.push_back(vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setDescriptorCount(inlineDescriptorData.dataSize).setDescriptorType(vk::DescriptorType::eInlineUniformBlockEXT).setPNext(&inlineDescriptorData));
        };

        vk::Device(*this->device).updateDescriptorSets(writes, {});
        return *this;
    };


    InputInterface& InputInterface::buildDescriptorSet() {
        // buffer view
        vk::BufferViewCreateInfo cpi{};
        cpi.format = vk::Format::eR8G8B8A8Uint; // for uniform texel view

        cpi.buffer = keysBufferInfo.buffer, cpi.offset = keysBufferInfo.offset, cpi.range = keysBufferInfo.range;
        keysBufferView = vk::Device(*device).createBufferView(cpi);

        cpi.buffer = swapBufferInfo.buffer, cpi.offset = swapBufferInfo.offset, cpi.range = swapBufferInfo.range;
        swapBufferView = vk::Device(*device).createBufferView(cpi);

        std::vector<vk::DescriptorSetLayout> dsLayouts = { device->getDescriptorSetLayoutSupport().at(1) };
        this->descriptorSet = vk::Device(*device).allocateDescriptorSets(vk::DescriptorSetAllocateInfo().setDescriptorPool(*device).setPSetLayouts(&dsLayouts[0]).setDescriptorSetCount(1)).at(0);

        // input data 
        const auto writeTmpl = vk::WriteDescriptorSet(this->descriptorSet, 0, 0, 1, vk::DescriptorType::eStorageBuffer);
        std::vector<vk::WriteDescriptorSet> writes = {
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo(&keysBufferInfo),
            vk::WriteDescriptorSet(writeTmpl).setDstBinding(0).setPBufferInfo(&swapBufferInfo).setDstArrayElement(1),
        };

        // inline descriptor 
        vk::WriteDescriptorSetInlineUniformBlockEXT inlineDescriptorData{};
        {
            inlineDescriptorData.dataSize = sizeof(uint32_t);
            inlineDescriptorData.pData = &this->elementCount;
            writes.push_back(vk::WriteDescriptorSet(writeTmpl).setDstBinding(6).setDescriptorCount(inlineDescriptorData.dataSize).setDescriptorType(vk::DescriptorType::eInlineUniformBlockEXT).setPNext(&inlineDescriptorData));
        };

        vk::Device(*this->device).updateDescriptorSets(writes, {});
        return *this;
    };


    VkResult Radix::initialize(const std::shared_ptr<radx::Device>& device) {
        this->device = device;
        std::vector<vk::DescriptorSetLayout> setLayouts = device->getDescriptorSetLayoutSupport();

        // push constant ranges
        vk::PushConstantRange pConstRange{};
        pConstRange.stageFlags = vk::ShaderStageFlagBits::eCompute;
        pConstRange.offset = 0u;
        pConstRange.size = sizeof(uint32_t) * 4u;

        // pipeline layout create info
        vk::PipelineLayoutCreateInfo pplLayoutCi{};
        pplLayoutCi.setLayoutCount = setLayouts.size();
        pplLayoutCi.pSetLayouts = setLayouts.data();
        pplLayoutCi.pushConstantRangeCount = 1;
        pplLayoutCi.pPushConstantRanges = &pConstRange;

        // create pipeline layout 
        this->pipelineLayout = vk::Device(*device).createPipelineLayout(pplLayoutCi);
        this->pipelines.push_back(createCompute(*device, device->getPath(radx::paths::counting   ), pipelineLayout, *device));
        this->pipelines.push_back(createCompute(*device, device->getPath(radx::paths::partition  ), pipelineLayout, *device));
        this->pipelines.push_back(createCompute(*device, device->getPath(radx::paths::scattering ), pipelineLayout, *device));
        this->pipelines.push_back(createCompute(*device, device->getPath(radx::paths::indiction  ), pipelineLayout, *device));
        this->pipelines.push_back(createCompute(*device, device->getPath(radx::paths::permutation), pipelineLayout, *device));

        // return shared_ptr when needed
        return VK_SUCCESS;
    };


    VkResult Radix::command(const vk::CommandBuffer& cmdBuf, const std::unique_ptr<radx::InternalInterface>& internalInterface, const std::shared_ptr<radx::InputInterface>& inputInterface, VkResult& vkres) {
        std::vector<vk::DescriptorSet> descriptors = { *internalInterface, *inputInterface };
        cmdBuf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, this->pipelineLayout, 0, descriptors, {});

        // radix sort phases
        const uint32_t stageCount = device->getDriverName() == "turing" ? 4u : 16u;
        for (auto I = 0u; I < stageCount; I++) { // TODO: add support variable stage length

            std::array<uint32_t, 4> stageC = { I,0,0,0 };
            cmdBuf.pushConstants(this->pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0u, sizeof(uint32_t) * 4, &stageC[0]);

            cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->pipelines[this->counting]);
            cmdBuf.dispatch(this->groupX, 1u, 1u);
            commandBarrier(cmdBuf);

            cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->pipelines[this->partition]);
            cmdBuf.dispatch(1u, 1u, 1u);
            commandBarrier(cmdBuf);

            cmdBuf.bindPipeline(vk::PipelineBindPoint::eCompute, this->pipelines[this->scattering]);
            cmdBuf.dispatch(this->groupX, 1u, 1u);
            commandBarrier(cmdBuf);

        };

        return VK_SUCCESS;
    };


    VkResult Radix::createInternalMemory(std::unique_ptr<radx::InternalInterface>& internalInterface, const size_t& maxElementCount) {
        vk::DeviceSize tileFix = 4u * this->groupX;

        vk::DeviceSize
            inlineSize = 0,//sizeof(uint32_t) * 4ull,
            keysSize = 256ull,//tiled(maxElementCount, tileFix) * tileFix * sizeof(uint32_t),
            keysBkpSize = 256ull,//tiled(maxElementCount, tileFix) * tileFix * sizeof(uint32_t),
            keyCacheSize = 256ull,//tiled(maxElementCount, tileFix) * tileFix * sizeof(uint32_t),
            referencesSize = 256ull * (this->groupX + 1) * sizeof(uint32_t),//tiled(maxElementCount, tileFix) * tileFix * sizeof(uint32_t),
            histogramsSize = referencesSize,
            prefixScanSize = histogramsSize
            ;

        vk::DeviceSize
            keysOffset = inlineSize,
            keysBkpOffset = keysOffset + keysSize,
            keyCacheOffset = keysBkpOffset + keysBkpSize,
            referencesOffset = keyCacheOffset + keyCacheSize,
            histogramsOffset = referencesOffset + referencesSize,
            prefixScanOffset = histogramsOffset + histogramsSize
            ;

        // get memory size and set max element count
        vk::DeviceSize memorySize = prefixScanOffset + prefixScanSize;
        internalInterface->setMaxElementCount(maxElementCount);

        // new keyed buffers
        internalInterface->setKeysStoreBufferInfo(vk::DescriptorBufferInfo(nullptr, keysOffset, keysSize));
        internalInterface->setKeysBackupBufferInfo(vk::DescriptorBufferInfo(nullptr, keysBkpOffset, keysBkpSize));
        internalInterface->setKeysCacheBufferInfo(vk::DescriptorBufferInfo(nullptr, keyCacheOffset, keyCacheSize));
        internalInterface->setCountMaximBufferInfo(vk::DescriptorBufferInfo(nullptr, referencesOffset, referencesSize)); // with work-group local prefix-scans

        // still required for effective sorting 
        internalInterface->setHistogramBufferInfo(vk::DescriptorBufferInfo(nullptr, histogramsOffset, histogramsSize));
        internalInterface->setPrefixScansBufferInfo(vk::DescriptorBufferInfo(nullptr, prefixScanOffset, prefixScanSize));

        // command for build descriptor set
        internalInterface->buildMemory(memorySize).buildDescriptorSet();

        return VK_SUCCESS;
    };
#endif

};
