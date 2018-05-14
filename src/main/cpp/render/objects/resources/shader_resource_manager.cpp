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
            .setStageFlags(vk::ShaderStageFlagBits::eVertex);

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
        if(pipeline_data.layouts.empty() || !pipeline_data.descriptors.empty()) {
            // If this pipeline already has descriptors, it shouldn't have any more
            return;
        }

        LOG(INFO) << "Creating descriptor sets for pipeline " << pipeline_data.name;

        auto layouts = std::vector<vk::DescriptorSetLayout>{};
        layouts.reserve(pipeline_data.layouts.size());

        for(uint32_t i = 0; i < pipeline_data.layouts.size(); i++) {
            layouts.push_back(pipeline_data.layouts.at(i));
        }
        auto alloc_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descriptor_pool)
            .setDescriptorSetCount(static_cast<uint32_t>(layouts.size()))
            .setPSetLayouts(layouts.data());

        pipeline_data.descriptors = device.allocateDescriptorSets(alloc_info);
        for(const auto& descriptor : pipeline_data.descriptors) {
            LOG(INFO) << "Allocated descriptor set " << (VkDescriptorSet)descriptor;
        }

        total_allocated_descriptor_sets += layouts.size();
        LOG(DEBUG) << "We've created " << total_allocated_descriptor_sets << " sets";
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
        LOG(INFO) << "Created point sampler " << (VkSampler)point_sampler;
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

        create_descriptor_pool(10000 + num_sets, 10000 + num_buffers, num_textures);

        for(auto &named_pipeline : pipelines) {
            for(auto& pipeline : named_pipeline.second) {
                create_descriptor_sets_for_pipeline(pipeline);
            }
        }
    }

    vk::DescriptorSet shader_resource_manager::create_model_matrix_descriptor() {
        LOG(DEBUG) << "Creating per-model descriptor " << ++per_model_descriptor_count;
        LOG(DEBUG) << "We've allocated " << ++total_allocated_descriptor_sets << " in total";
        auto descriptor = device.allocateDescriptorSets(model_matrix_descriptor_allocate_info)[0];
        LOG(INFO) << "Allocated descriptor " << (VkDescriptorSet)descriptor;
        return descriptor;
    }

    void shader_resource_manager::free_descriptor(vk::DescriptorSet to_free) {
        LOG(INFO) << "Freeing descriptor " << (VkDescriptorSet)to_free;
        per_model_descriptor_count--;
        total_allocated_descriptor_sets--;
        device.freeDescriptorSets(descriptor_pool, {to_free});
    }
}

