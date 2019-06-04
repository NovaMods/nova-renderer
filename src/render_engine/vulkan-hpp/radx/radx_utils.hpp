#pragma once

#include "radx_core.hpp"

namespace radx {

    template <typename T>
    static inline auto sgn(const T& val) { return (T(0) < val) - (val < T(0)); }

    template<class T = uint64_t>
    static inline T tiled(const T& sz, const T& gmaxtile) {
        // return (int32_t)ceil((double)sz / (double)gmaxtile);
        return sz <= 0 ? 0 : (sz / gmaxtile + sgn(sz % gmaxtile));
    }

    template <class T>
    static inline auto strided(const size_t& sizeo) { return sizeof(T) * sizeo; }

    // read binary (for SPIR-V)
    static inline auto readBinary(const std::string& filePath ) {
        std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
        std::vector<uint32_t> data = {};
        if (file.is_open()) {
            std::streampos size = file.tellg();
            data.resize(tiled(size_t(size), sizeof(uint32_t)));
            file.seekg(0, std::ios::beg);
            file.read((char *)data.data(), size);
            file.close();
        } else {
            std::cerr << "Failure to open " + filePath << std::endl;
        }
        return data;
    };

    // read source (unused)
    static inline auto readSource(const std::string& filePath, bool lineDirective = false ) {
        std::string content = "";
        std::ifstream fileStream(filePath, std::ios::in);
        if (!fileStream.is_open()) {
            std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl; return content;
        }
        std::string line = "";
        while (!fileStream.eof()) {
            std::getline(fileStream, line);
            if (lineDirective || line.find("#line") == std::string::npos) content.append(line + "\n");
        }
        fileStream.close();
        return content;
    };

    static inline auto makeShaderModuleInfo(const std::vector<uint32_t>& code) {
        auto smi = vk::ShaderModuleCreateInfo{};
        smi.pCode = (uint32_t *)code.data();
        smi.codeSize = code.size()*4;
        smi.flags = {};
        return smi;
    };

    // create shader module
    static inline auto createShaderModuleIntrusive(const vk::Device& device, const std::vector<uint32_t>& code, vk::ShaderModule& hndl) {
        return (hndl = device.createShaderModule(makeShaderModuleInfo(code)));
    };

    static inline auto createShaderModule(const vk::Device& device, const std::vector<uint32_t>& code) {
        auto sm = vk::ShaderModule{}; return createShaderModuleIntrusive(device, code, sm); return sm;
    };

    // create shader module
    static inline auto makeComputePipelineStageInfo(const vk::Device& device, const std::vector<uint32_t>& code, const char * entry = "main") {
        auto spi = vk::PipelineShaderStageCreateInfo{};
        spi.flags = {};
        createShaderModuleIntrusive(device, code, spi.module);
        spi.pName = entry;
        spi.stage = vk::ShaderStageFlagBits::eCompute;
        spi.pSpecializationInfo = {};
        return spi;
    };

    // create compute pipelines
    static inline auto createCompute(const vk::Device& device, const vk::PipelineShaderStageCreateInfo& spi, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}) {
        auto cmpi = vk::ComputePipelineCreateInfo{};
        cmpi.flags = {};
        cmpi.layout = layout;
        cmpi.stage = spi;
        cmpi.basePipelineIndex = -1;
        return device.createComputePipeline(cache, cmpi);
    };

    // create compute pipelines
    static inline auto createCompute(const vk::Device& device, const std::vector<uint32_t>& code, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}) {
        return createCompute(device, makeComputePipelineStageInfo(device, code), layout, cache);
    };

    // create compute pipelines
    static inline auto createCompute(const vk::Device& device, const std::string& path, const vk::PipelineLayout& layout, const vk::PipelineCache& cache = {}) {
        return createCompute(device, readBinary(path), layout, cache);
    };

    // general command buffer pipeline barrier
    static inline void commandBarrier(const vk::CommandBuffer& cmdBuffer) {
        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.pNext = nullptr;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; //| VK_ACCESS_TRANSFER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT;
        cmdBuffer.pipelineBarrier(
            //vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eTransfer | vk::PipelineStageFlagBits::eComputeShader, {}, { memoryBarrier }, {}, {});
    };


	// create secondary command buffers for batching compute invocations
	static inline auto createCommandBuffer(VkDevice device, VkCommandPool cmdPool, bool secondary = true, bool once = true) {
		VkCommandBuffer cmdBuffer = {};

		VkCommandBufferAllocateInfo cmdi = vk::CommandBufferAllocateInfo{};
		cmdi.commandPool = cmdPool;
		cmdi.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdi.commandBufferCount = 1;
		vkAllocateCommandBuffers(device, &cmdi, &cmdBuffer);

		VkCommandBufferInheritanceInfo inhi = vk::CommandBufferInheritanceInfo{};
		inhi.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

		VkCommandBufferBeginInfo bgi = vk::CommandBufferBeginInfo{};
		bgi.flags = {};
		bgi.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		bgi.pInheritanceInfo = secondary ? &inhi : nullptr;
		vkBeginCommandBuffer(cmdBuffer, &bgi);

		return cmdBuffer;
	};

	// add dispatch in command buffer (with default pipeline barrier)
	static inline VkResult cmdDispatch(VkCommandBuffer cmd, VkPipeline pipeline, uint32_t x = 1, uint32_t y = 1, uint32_t z = 1, bool barrier = true) {
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		vkCmdDispatch(cmd, x, y, z);
		if (barrier) {
			commandBarrier(cmd); // put shader barrier
		}
		return VK_SUCCESS;
	};

	// low level copy command between (prefer for host and device)
	static inline VkResult cmdCopyBufferL(VkCommandBuffer cmd, VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<vk::BufferCopy>& regions, std::function<void(VkCommandBuffer)> barrierFn = commandBarrier) {
		if (srcBuffer && dstBuffer && regions.size() > 0) {
			vk::CommandBuffer(cmd).copyBuffer(srcBuffer, dstBuffer, regions); barrierFn(cmd); // put copy barrier
		};
		return VK_SUCCESS;
	};


	// short data set with command buffer (alike push constant)
	template<class T>
	static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize offset, const std::vector<T>& data) {
		vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, data);
		//updateCommandBarrier(cmd);
		return VK_SUCCESS;
	};

	// short data set with command buffer (alike push constant)
	template<class T>
	static inline VkResult cmdUpdateBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const T* data) {
		vk::CommandBuffer(cmd).updateBuffer(dstBuffer, offset, size, data);
		//updateCommandBarrier(cmd);
		return VK_SUCCESS;
	};


	// template function for fill buffer by constant value
	// use for create repeat variant
	template<uint32_t Rv>
	static inline VkResult cmdFillBuffer(VkCommandBuffer cmd, VkBuffer dstBuffer, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		vk::CommandBuffer(cmd).fillBuffer(vk::Buffer(dstBuffer), offset, size, Rv);
		//updateCommandBarrier(cmd);
		return VK_SUCCESS;
	};


	// submit command (with async wait)
	static inline void submitCmd(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, vk::SubmitInfo smbi = {}) {
		// no commands 
		if (cmds.size() <= 0) return;

		smbi.commandBufferCount = cmds.size();
		smbi.pCommandBuffers = (vk::CommandBuffer*)cmds.data();

		VkFence fence = {}; VkFenceCreateInfo fin = vk::FenceCreateInfo{};
		vkCreateFence(device, &fin, nullptr, &fence);
		vkQueueSubmit(queue, 1, (const VkSubmitInfo*)& smbi, fence);
		vkWaitForFences(device, 1, &fence, true, INT64_MAX);
		vkDestroyFence(device, fence, nullptr);
	};

	// once submit command buffer
	static inline void submitOnce(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, vk::SubmitInfo smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); vkEndCommandBuffer(cmdBuf);
		submitCmd(device, queue, { cmdBuf }); vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
	};

	// submit command (with async wait)
	static inline void submitCmdAsync(VkDevice device, VkQueue queue, std::vector<VkCommandBuffer> cmds, std::function<void()> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
		// no commands 
		if (cmds.size() <= 0) return;

		smbi.commandBufferCount = cmds.size();
		smbi.pCommandBuffers = (const vk::CommandBuffer*)cmds.data();

		VkFence fence = {}; VkFenceCreateInfo fin = vk::FenceCreateInfo{};
		vkCreateFence(device, &fin, nullptr, &fence);
		vkQueueSubmit(queue, 1, (const VkSubmitInfo*)& smbi, fence);
		vkWaitForFences(device, 1, &fence, true, INT64_MAX);
		vkDestroyFence(device, fence, nullptr);
		if (asyncCallback) asyncCallback();
	};

	// once submit command buffer
	static inline void submitOnceAsync(VkDevice device, VkQueue queue, VkCommandPool cmdPool, std::function<void(VkCommandBuffer)> cmdFn = {}, std::function<void(VkCommandBuffer)> asyncCallback = {}, vk::SubmitInfo smbi = {}) {
		auto cmdBuf = createCommandBuffer(device, cmdPool, false); cmdFn(cmdBuf); vkEndCommandBuffer(cmdBuf);
		submitCmdAsync(device, queue, { cmdBuf }, [&]() {
			asyncCallback(cmdBuf); // call async callback
			vkFreeCommandBuffers(device, cmdPool, 1, &cmdBuf); // free that command buffer
		});
	};

	template <class T> static inline auto makeVector(const T * ptr, size_t size = 1) { std::vector<T>v(size); memcpy(v.data(), ptr, strided<T>(size)); return v; };

	// create fence function
	static inline vk::Fence createFence(VkDevice device, bool signaled = true) {
		vk::FenceCreateInfo info = {};
		if (signaled) info.setFlags(vk::FenceCreateFlagBits::eSignaled);
		return vk::Device(device).createFence(info);
	};



};
