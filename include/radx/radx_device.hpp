#pragma once 

#include "radx_core.hpp"
#include "radx_shaders.hpp"

// TODO: 
// - getting features and properties
// - getting vendor naming
// - detecting what is GPU

namespace radx {

    class PhysicalDeviceHelper : public std::enable_shared_from_this<PhysicalDeviceHelper> {
    protected:
        vk::PhysicalDevice physicalDevice = {};
        vk::PhysicalDeviceFeatures2 features = {};
        vk::PhysicalDeviceProperties2 properties = {};
        std::vector<uint32_t> queueFamilyIndices = {};
        VmaAllocator allocator = {};

        // required (if there is no, will generated)
        std::shared_ptr<paths::DriverWrapBase> driverWrap = {};
        
        virtual VkResult getFeaturesWithProperties(){
            this->features = physicalDevice.getFeatures2();
            this->properties = physicalDevice.getProperties2();
            return VK_SUCCESS;
        };

        virtual VkResult getVendorName(){
            driverWrap = paths::getNamedDriver(this->properties.properties.vendorID, this->features.features.shaderInt16);
            return VK_SUCCESS;
        };

    public:
        friend radx::Device;

        // require to generate both VMA and vendor name 
        PhysicalDeviceHelper(const vk::PhysicalDevice& physicalDevice) : physicalDevice(physicalDevice) {
            this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
        };

        // require vendor name 
        PhysicalDeviceHelper(const vk::PhysicalDevice& physicalDevice, const VmaAllocator& allocator) : physicalDevice(physicalDevice), allocator(allocator) {
            this->physicalDevice = physicalDevice, this->getFeaturesWithProperties(), this->getVendorName();
            this->allocator = allocator;
        };

        // getter of vendor name 
        operator const std::shared_ptr<paths::DriverWrapBase>&() const { return driverWrap; };
        std::string getPath(const std::string fpath) const { return driverWrap->getPath(fpath); };
        std::string getDriverName() const { return driverWrap->getDriverName(); };

        // vk::PhysicalDevice caster
        operator vk::PhysicalDevice&() { return physicalDevice; };
        operator const vk::PhysicalDevice&() const { return physicalDevice; };


        operator VkPhysicalDevice&() { return (VkPhysicalDevice&)physicalDevice; };
        operator const VkPhysicalDevice&() const { return (VkPhysicalDevice&)physicalDevice; };
    };

    class Device : public std::enable_shared_from_this<Device> {
    protected:
        vk::Device device;
        
        // descriptor set layout 
        vk::DescriptorPool descriptorPool = {}; vk::PipelineCache pipelineCache = {};
        //vk::DescriptorSetLayout sortInputLayout, sortInterfaceLayout;

        std::vector<vk::DescriptorSetLayout> descriptorLayouts = {};
        uint32_t sortInput = 1, sortInterface = 0;

        std::shared_ptr<radx::PhysicalDeviceHelper> physicalHelper;
        VmaAllocator allocator = {};

    public:
        ~Device() {
            device.waitIdle(); // wait idle before than device has been destroyed
            vmaDestroyAllocator(allocator);
        };

        const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayoutSupport() const { return descriptorLayouts; };
        std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayoutSupport() { return descriptorLayouts; };

        std::shared_ptr<Device> setDescriptorPool(const vk::DescriptorPool& descriptorPool) {this->descriptorPool = descriptorPool; return shared_from_this(); };
        std::shared_ptr<Device> initialize(const vk::Device& device, std::shared_ptr<radx::PhysicalDeviceHelper> physicalHelper);

        // queue family indices
        std::vector<uint32_t>& queueFamilyIndices() {return physicalHelper->queueFamilyIndices;};
        const std::vector<uint32_t>& queueFamilyIndices() const {return physicalHelper->queueFamilyIndices;};

        // getter of shared_ptr physical device helper
        operator std::shared_ptr<radx::PhysicalDeviceHelper>&(){ return physicalHelper; };
        operator const std::shared_ptr<radx::PhysicalDeviceHelper>&() const { return physicalHelper; };

        // get physical device helper
        std::shared_ptr<radx::PhysicalDeviceHelper>& getPhysicalHelper(){ return *this; };
        const std::shared_ptr<radx::PhysicalDeviceHelper>& getPhysicalHelper() const { return *this; };

        // 
        operator const std::shared_ptr<paths::DriverWrapBase>& () const { return *physicalHelper; };
        std::string getPath(const std::string fpath) const { return physicalHelper->getPath(fpath); };
        std::string getDriverName() const { return physicalHelper->getDriverName(); }

        // vk::PhysicalDevice caster
        operator vk::PhysicalDevice& () { return *physicalHelper; };
        operator const vk::PhysicalDevice& () const { return *physicalHelper; };

        // vk::DescriptorPool caster
        operator vk::DescriptorPool&() { return descriptorPool; };
        operator const vk::DescriptorPool&() const { return descriptorPool; };

        // vk::PipelineCache caster
        operator vk::PipelineCache&() { return pipelineCache; };
        operator const vk::PipelineCache&() const { return pipelineCache; };

        // vk::Device caster
        operator vk::Device&() { return device; };
        operator const vk::Device&() const { return device; };

        // vk::Device caster
        operator VkDevice&() { return (VkDevice&)device; };
        operator const VkDevice&() const { return (VkDevice&)device; };

        // VmaAllocator caster
        operator VmaAllocator&() { return allocator; };
        operator const VmaAllocator&() const { return allocator; };
    };
};
