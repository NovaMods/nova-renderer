/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#include <easylogging++.h>
#include "shader_resource_manager.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"

namespace nova {
    shader_resource_manager::shader_resource_manager(std::shared_ptr<render_context> context) : device(context->device), context(context) {
        auto per_model_buffer_create_info = vk::BufferCreateInfo()
                .setSize(5000 * sizeof(glm::mat4))
                .setUsage(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndexCount(1)
                .setPQueueFamilyIndices(&context->graphics_family_idx);

        auto uniform_buffer_offset_alignment = context->gpu.props.limits.minUniformBufferOffsetAlignment;
        per_model_resources_buffer = std::make_shared<auto_buffer>(context, per_model_buffer_create_info, uniform_buffer_offset_alignment, true);
    }

    void shader_resource_manager::create_descriptor_pool(uint32_t num_sets, uint32_t num_buffers, uint32_t num_textures) {
        vk::DescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.maxSets = num_sets;
        pool_create_info.poolSizeCount = 2;

        vk::DescriptorPoolSize sizes[] = {
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(num_textures),
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(num_buffers),
        };

        pool_create_info.pPoolSizes = sizes;
        pool_create_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        descriptor_pool = device.createDescriptorPool(pool_create_info);
    }

    shader_resource_manager::~shader_resource_manager() {
        device.destroyDescriptorPool(descriptor_pool);

        device.destroySampler(point_sampler);
    }

    std::shared_ptr<auto_buffer> shader_resource_manager::get_per_model_buffer() {
        return per_model_resources_buffer;
    }

    void shader_resource_manager::create_descriptor_sets_for_pipeline(pipeline_object &pipeline_data) {
        auto layouts = std::vector<vk::DescriptorSetLayout>{};
        layouts.reserve(pipeline_data.layouts.size());

        for(uint32_t i = 0; i < pipeline_data.layouts.size(); i++) {
            layouts[i] = pipeline_data.layouts[i];
        }

        auto alloc_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descriptor_pool)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

        pipeline_data.descriptors = device.allocateDescriptorSets(alloc_info);
    }

    void shader_resource_manager::create_point_sampler() {
        auto sampler_create = vk::SamplerCreateInfo()
            .setMagFilter(vk::Filter::eNearest)
            .setMinFilter(vk::Filter::eNearest)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
            .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
            .setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
            .setMipLodBias(0)
            .setAnisotropyEnable(false)
            .setCompareEnable(false)
            .setMinLod(0)
            .setMaxLod(32)
            .setUnnormalizedCoordinates(false)
            .setBorderColor(vk::BorderColor::eFloatTransparentBlack);

        point_sampler = device.createSampler(sampler_create);
    }

    vk::Sampler shader_resource_manager::get_point_sampler() {
        return point_sampler;
    }
}

