/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#include "shader_resource_manager.h"
#include "../../vulkan/render_context.h"

namespace nova {

    shader_resource_manager::shader_resource_manager() : device(render_context::instance.device) {
        create_block_textures_dsl();
        create_custom_textures_dsl();
        create_shadow_textures_dsl();
        create_depth_textures_dsl();
        create_noise_tex_dsl();
        create_framebuffer_top_dsl();
        create_framebuffer_bottom_dsl();
        create_block_light_dsl();

        create_descriptor_pool();
    }

    void shader_resource_manager::create_block_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        block_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_custom_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(7).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(8).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(9).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(10).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        custom_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_shadow_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(4).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(5).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(13).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(14).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        shadow_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_depth_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(6).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(11).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(12).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 3;
        create_info.pBindings = bindings;

        depth_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_noise_tex_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(15).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 1;
        create_info.pBindings = bindings;

        noise_tex_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_framebuffer_top_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        framebuffer_top_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_framebuffer_bottom_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(7).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(8).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(9).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(10).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        framebuffer_bottom_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_block_light_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(4).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(16).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        block_light_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_block_textures_ds() {
        vk::DescriptorSetLayout layouts[] = {

        };

        auto alloc_info = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descriptor_pool)
            .setDescriptorSetCount(4)
            .setPSetLayouts(nullptr);
    }

    void shader_resource_manager::create_descriptor_pool() {
        vk::DescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.maxSets = 32;  // Nova hopefully won't need too many
        pool_create_info.poolSizeCount = 3;

        // TODO: Tune these values for actual usage needs
        vk::DescriptorPoolSize sizes[] = {
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eSampledImage).setDescriptorCount(9),
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eInputAttachment).setDescriptorCount(16),
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(2),
        };

        pool_create_info.pPoolSizes = sizes;

        descriptor_pool = device.createDescriptorPool(pool_create_info);
    }
}

