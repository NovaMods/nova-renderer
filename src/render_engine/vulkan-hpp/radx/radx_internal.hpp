#pragma once 

#include "radx_core.hpp"
#include "radx_device.hpp"
#include "radx_buffer.hpp"

namespace radx {

    class InternalInterface { // used for connection between algorithms and storage
    protected:
        std::shared_ptr<radx::Device> device = {};
        std::unique_ptr<VmaAllocatedBuffer> bufferMemory = {}; // allocated personally, once
        

        vk::DescriptorSet descriptorSet = {};
        size_t maxElementCount = 1024*1024;

    public:
        friend Algorithm;
        InternalInterface(){};
        InternalInterface(const std::shared_ptr<radx::Device>& device): device(device) {
            
        };

        // TODO: use std::vector instead of 
        vk::DescriptorBufferInfo 
            keysStoreBufferInfo = {}, keysBackupBufferInfo = {},
            keysCacheBufferInfo = {}, countMaximBufferInfo = {},
            histogramBufferInfo = {}, prefixScansBufferInfo = {};

        // deprecated stuff
        virtual InternalInterface& setKeysStoreBufferInfo(const vk::DescriptorBufferInfo& keysStore = {}) { this->keysStoreBufferInfo = keysStore; return *this; };
        virtual InternalInterface& setKeysBackupBufferInfo(const vk::DescriptorBufferInfo& keysBackup = {}) { this->keysBackupBufferInfo = keysBackup; return *this; };
        virtual InternalInterface& setKeysCacheBufferInfo(const vk::DescriptorBufferInfo& keysCache = {}){ this->keysCacheBufferInfo = keysCache; return *this; };

        // current cache stuff
        virtual InternalInterface& setCountMaximBufferInfo(const vk::DescriptorBufferInfo& references = {}){ this->countMaximBufferInfo = references; return *this; };
        virtual InternalInterface& setHistogramBufferInfo(const vk::DescriptorBufferInfo& histogram = {}){ this->histogramBufferInfo = histogram; return *this; };
        virtual InternalInterface& setPrefixScansBufferInfo(const vk::DescriptorBufferInfo& prefixScans = {}){ this->prefixScansBufferInfo = prefixScans; return *this; };
        virtual InternalInterface& setMaxElementCount(const size_t& elementCount = 0) { this->maxElementCount = maxElementCount; return *this; };
        virtual InternalInterface& buildMemory(const vk::DeviceSize& memorySize) {this->bufferMemory = std::make_unique<radx::VmaAllocatedBuffer>(this->device, memorySize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformTexelBuffer); return *this; };
        virtual InternalInterface& buildDescriptorSet();

        // vk::DescriptorSet caster
        operator vk::DescriptorSet&() { return descriptorSet; };
        operator const vk::DescriptorSet&() const { return descriptorSet; };
    };


    class InputInterface {
    protected:
        std::shared_ptr<radx::Device> device = {};

        // TODO: use std::vector instead of 
        vk::DescriptorSet descriptorSet;
        vk::BufferView keysBufferView = {}, swapBufferView = {};

    public:
        friend Algorithm;
        InputInterface(){};
        InputInterface(const std::shared_ptr<radx::Device>& device): device(device) {};
        vk::DescriptorBufferInfo keysBufferInfo = {}, swapBufferInfo = {};
        size_t elementCount = 0;

        // for building arguments 
        virtual InputInterface& setKeysBufferInfo(const vk::DescriptorBufferInfo& keys = {}){ this->keysBufferInfo = keys; return *this; };
        virtual InputInterface& setSwapBufferInfo(const vk::DescriptorBufferInfo& swap = {}){ this->swapBufferInfo = swap; return *this; };
        virtual InputInterface& setElementCount(const size_t& elementCount = 0) { this->elementCount = elementCount; return *this; };
        virtual InputInterface& buildDescriptorSet();

        // vk::DescriptorSet caster
        operator vk::DescriptorSet&() { return descriptorSet; };
        operator const vk::DescriptorSet&() const { return descriptorSet; };
    };


    // abstract class for sorting alrgorithm
    class Algorithm : public std::enable_shared_from_this<Algorithm> {
    protected:
        std::shared_ptr<radx::Device> device;

        uint32_t
            groupX = 1,//64,
            groupY = 1;
        std::vector<vk::Pipeline> pipelines = {};
        
        vk::PipelineLayout pipelineLayout;

        // internal methods (for devs)
        virtual VkResult command(const vk::CommandBuffer& cmdBuf, const std::unique_ptr<radx::InternalInterface>& internalInterface, const std::shared_ptr<radx::InputInterface>& inputInterface, VkResult& vkres) { return VK_SUCCESS; };
        virtual VkResult createInternalMemory(std::unique_ptr<radx::InternalInterface>& internalInterface, const size_t& maxElementCount = 1024 * 1024) { return VK_SUCCESS; };

    public:
        Algorithm(): groupX(1){};

        friend Sort<Algorithm>;
        virtual VkResult initialize(const std::shared_ptr<radx::Device>& device) { return VK_SUCCESS; };

        // can be used by children 
        virtual operator Algorithm&() { return *this; };
        virtual operator const Algorithm&() const { return *this; };
    };


    template <class T>
    class Sort : public std::enable_shared_from_this<Sort<T>> {
    protected:
        std::shared_ptr<T> algorithm;
        std::shared_ptr<radx::Device> device;
        std::unique_ptr<radx::InternalInterface> internalInterface;
        //std::shared_ptr<radx::InputInterface> inputInterface;
        
    public:

        // 
        virtual Sort<T>& initialize(const std::shared_ptr<radx::Device>& device, const std::shared_ptr<T>& algorithm, const size_t& maxElementCount = 1024 * 1024) {
            this->device = device, this->algorithm = algorithm;
            this->algorithm->createInternalMemory(this->internalInterface = std::make_unique<InternalInterface>(this->device), maxElementCount);
            return *this;
        };

        // accepts only right-based links 
        virtual Sort<T>& initialize(const std::shared_ptr<radx::Device>& device, std::shared_ptr<radx::Algorithm>&& algorithm, const size_t& maxElementCount = 1024 * 1024) {
            this->initialize(device, std::move(std::dynamic_pointer_cast<T>(std::move(algorithm))), maxElementCount);
            return *this;
        };


        // TODO: add unique ptr support of input interface 
        virtual Sort<T>& command(const vk::CommandBuffer& cmdBuf, std::shared_ptr<radx::InputInterface>& inputInterface){
            VkResult vkres = VK_SUCCESS; algorithm->command(cmdBuf, internalInterface, inputInterface, vkres); //return vkres;
            return *this;
        };

    };

    // TODO: better vendor-based setup for device 
    class Radix : public Algorithm, public std::enable_shared_from_this<Radix> {
    protected:
        uint32_t counting = 0, partition = 1, scattering = 2, indiction = 3, permutation = 4, resolve = 5;
        virtual VkResult command(const vk::CommandBuffer& cmdBuf, const std::unique_ptr<radx::InternalInterface>& internalInterface, const std::shared_ptr<radx::InputInterface>& inputInterface, VkResult& vkres) override;

    public:
        Radix() { this->groupX = 72u; };

        friend Sort<Radix>;
        virtual VkResult initialize(const std::shared_ptr<radx::Device>& device) override;
        virtual VkResult createInternalMemory(std::unique_ptr<radx::InternalInterface>& internalInterface, const size_t& maxElementCount = 1024 * 1024) override;
    };

};
