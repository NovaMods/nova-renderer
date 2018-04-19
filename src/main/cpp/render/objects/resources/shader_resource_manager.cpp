/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#include <easylogging++.h>
#include "shader_resource_manager.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"

namespace nova {
    shader_resource_manager::shader_resource_manager(std::shared_ptr<render_context> context) : device(context->device), textures(context), buffers(context), context(context) {
        create_point_sampler();
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

        auto model_matrix_descriptor_binding = vk::DescriptorSetLayoutBinding()
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eAll);

        auto model_matrix_descriptor_create_info = vk::DescriptorSetLayoutCreateInfo()
            .setBindingCount(1)
            .setPBindings(&model_matrix_descriptor_binding);

        model_matrix_layout = device.createDescriptorSetLayout({model_matrix_descriptor_create_info});

        model_matrix_descriptor_allocate_info = vk::DescriptorSetAllocateInfo()
                    .setDescriptorPool(descriptor_pool)
                    .setDescriptorSetCount(1)
                    .setPSetLayouts(&model_matrix_layout);
    }

    shader_resource_manager::~shader_resource_manager() {
        device.destroyDescriptorPool(descriptor_pool);

        device.destroySampler(point_sampler);
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

    texture_manager &shader_resource_manager::get_texture_manager() {
        return textures;
    }

    uniform_buffer_store &shader_resource_manager::get_uniform_buffers() {
        return buffers;
    }

    void shader_resource_manager::create_descriptor_sets(std::unordered_map<std::string, std::vector<pipeline_object>> &pipelines) {
        uint32_t num_sets = 0, num_textures = 0, num_buffers = 0;

        for(const auto &named_pipeline : pipelines) {
            for(const auto &pipeline : named_pipeline.second) {

                num_sets += pipeline.layouts.size();

                for(const auto &named_binding : pipeline.resource_bindings) {
                    const resource_binding &binding = named_binding.second;

                    if(binding.descriptorType == vk::DescriptorType::eUniformBuffer) {
                        num_buffers++;

                    } else if(binding.descriptorType == vk::DescriptorType::eCombinedImageSampler) {
                        num_textures++;

                    } else {
                        LOG(WARNING) << "Descriptor type " << vk::to_string(binding.descriptorType)
                                     << " is not supported yet";
                    }
                }
            }
        }

        create_descriptor_pool(num_sets, num_buffers, num_textures);

        for(auto &named_pipeline : pipelines) {
            for(auto& pipeline : named_pipeline.second) {
                create_descriptor_sets_for_pipeline(pipeline);
            }
        }
    }

    vk::DescriptorSet shader_resource_manager::create_model_matrix_descriptor() {
        return device.allocateDescriptorSets(model_matrix_descriptor_allocate_info)[0];
    }

    void shader_resource_manager::free_descriptor(vk::DescriptorSet to_free) {
        device.freeDescriptorSets(descriptor_pool, {to_free});
    }

}

