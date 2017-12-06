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
        create_common_ds();
        create_framebuffer_top_dsl();
        create_framebuffer_bottom_dsl();
        create_block_light_dsl();

        create_pipeline_layouts();

        create_descriptor_pool();

        create_desriptor_sets();
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

    void shader_resource_manager::create_common_ds() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(15).setDescriptorType(vk::DescriptorType::eSampledImage).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(16).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        common_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_per_model_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(17).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 1;
        create_info.pBindings = bindings;

        per_model_dsl = device.createDescriptorSetLayout(create_info);
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


    void shader_resource_manager::create_pipeline_layouts() {
        vk::DescriptorSetLayout shadow_set_layouts[] = {
                block_textures_dsl,
                common_dsl,
                per_model_dsl,
                custom_textures_dsl,
        };

        vk::PipelineLayoutCreateInfo shadow_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(4)
            .setPSetLayouts(shadow_set_layouts);

        shadow_pl = device.createPipelineLayout(shadow_pl_create_info);


        vk::DescriptorSetLayout gbuffers_set_layouts[] = {
                block_textures_dsl,
                shadow_textures_dsl,
                common_dsl,
                per_model_dsl,
                custom_textures_dsl,
        };

        auto gbuffers_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(5)
            .setPSetLayouts(gbuffers_set_layouts);

        gbuffers_pl = device.createPipelineLayout(gbuffers_pl_create_info);


        vk::DescriptorSetLayout deferred_light_set_layouts[] = {
                depth_textures_dsl,
                common_dsl,
                per_model_dsl,
                block_light_dsl
        };

        auto deferred_light_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(4)
            .setPSetLayouts(deferred_light_set_layouts);

        deferred_light_pl = device.createPipelineLayout(deferred_light_pl_create_info);


        vk::DescriptorSetLayout fullscreen_pass_layouts[] = {
                shadow_textures_dsl,
                depth_textures_dsl,
                common_dsl,
                per_model_dsl,
                framebuffer_top_dsl,
                framebuffer_bottom_dsl,
        };

        auto fullscreen_pass_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(6)
            .setPSetLayouts(fullscreen_pass_layouts);

        fullscreen_pass_pl = device.createPipelineLayout(fullscreen_pass_pl_create_info);
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

    void shader_resource_manager::create_desriptor_sets() {
        vk::DescriptorSetLayout layouts[] = {
            block_textures_dsl,
            custom_textures_dsl,
            shadow_textures_dsl,
            depth_textures_dsl,
            common_dsl,
            framebuffer_top_dsl,
            framebuffer_bottom_dsl,
            block_light_dsl,
            per_model_dsl,
        };

        auto alloc_info = vk::DescriptorSetAllocateInfo()
                        .setDescriptorPool(descriptor_pool)
                        .setDescriptorSetCount(8)
                        .setPSetLayouts(layouts);

        auto descriptor_sets = device.allocateDescriptorSets(alloc_info);

        block_textures          = descriptor_sets[0];
        custom_textures         = descriptor_sets[1];
        shadow_textures         = descriptor_sets[2];
        depth_textures          = descriptor_sets[3];
        common_descriptors      = descriptor_sets[4];
        framebuffer_top         = descriptor_sets[5];
        framebuffer_bottom      = descriptor_sets[6];
        block_light             = descriptor_sets[7];
        per_model_descriptors   = descriptor_sets[8];
    }


    shader_resource_manager::~shader_resource_manager() {
        device.destroyDescriptorPool(descriptor_pool);
    }
}

