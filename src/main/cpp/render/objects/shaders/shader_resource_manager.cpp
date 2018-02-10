/*!
 * \author ddubois 
 * \date 03-Dec-17.
 */

#include <easylogging++.h>
#include "shader_resource_manager.h"
#include "../../vulkan/render_context.h"

namespace nova {

    std::shared_ptr<shader_resource_manager> shader_resource_manager::instance;

    shader_resource_manager::shader_resource_manager() : device(render_context::instance.device) {
        create_block_textures_dsl();
        create_custom_textures_dsl();
        create_shadow_textures_dsl();
        create_depth_textures_dsl();
        create_common_ds();
        create_framebuffer_top_dsl();
        create_framebuffer_bottom_dsl();
        create_block_light_dsl();
        create_per_model_dsl();

        create_pipeline_layouts();

        create_descriptor_pool();

        create_descriptor_sets();
    }

    void shader_resource_manager::create_block_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        block_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_custom_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        custom_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_shadow_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        shadow_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_depth_textures_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 3;
        create_info.pBindings = bindings;

        depth_textures_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_common_ds() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        common_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_per_model_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 1;
        create_info.pBindings = bindings;

        per_model_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_framebuffer_top_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        framebuffer_top_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_framebuffer_bottom_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 4;
        create_info.pBindings = bindings;

        framebuffer_bottom_dsl = device.createDescriptorSetLayout(create_info);
    }

    void shader_resource_manager::create_block_light_dsl() {
        vk::DescriptorSetLayoutBinding bindings[] = {
                vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
                vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setStageFlags(vk::ShaderStageFlagBits::eAll),
        };

        vk::DescriptorSetLayoutCreateInfo create_info = {};
        create_info.bindingCount = 2;
        create_info.pBindings = bindings;

        block_light_dsl = device.createDescriptorSetLayout(create_info);
    }


    void shader_resource_manager::create_pipeline_layouts() {
        auto max_bound_descriptor_sets = render_context::instance.gpu.props.limits.maxBoundDescriptorSets;
        if(max_bound_descriptor_sets < 7) {
            LOG(FATAL) << "We need 7 descriptor sets at a time, but your system only supports " << max_bound_descriptor_sets;
        }

        vk::DescriptorSetLayout shadow_set_layouts[] = {
                common_dsl,
                per_model_dsl,
                block_textures_dsl,
                custom_textures_dsl,
        };

        vk::PipelineLayoutCreateInfo shadow_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(4)
            .setPSetLayouts(shadow_set_layouts);

        layouts[pass_enum::Shadow] = device.createPipelineLayout(shadow_pl_create_info);


        vk::DescriptorSetLayout gbuffers_set_layouts[] = {
                common_dsl,
                per_model_dsl,
                block_textures_dsl,
                custom_textures_dsl,
                shadow_textures_dsl,
        };

        auto gbuffers_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(5)
            .setPSetLayouts(gbuffers_set_layouts);

        layouts[pass_enum::Gbuffer] = device.createPipelineLayout(gbuffers_pl_create_info);


        vk::DescriptorSetLayout transparent_set_layout[] = {
                common_dsl,
                per_model_dsl,
                block_textures_dsl,
                custom_textures_dsl,
                shadow_textures_dsl,
                framebuffer_bottom_dsl,
                depth_textures_dsl,
        };

        auto transparenty_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(7)
            .setPSetLayouts(transparent_set_layout);

        layouts[pass_enum::Transparent] = device.createPipelineLayout(transparenty_pl_create_info);


        vk::DescriptorSetLayout deferred_light_set_layouts[] = {
                common_dsl,
                per_model_dsl,
                framebuffer_top_dsl,
                framebuffer_bottom_dsl,
                depth_textures_dsl,
                block_light_dsl
        };

        auto deferred_light_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(6)
            .setPSetLayouts(deferred_light_set_layouts);

        layouts[pass_enum::DeferredLight] = device.createPipelineLayout(deferred_light_pl_create_info);


        vk::DescriptorSetLayout fullscreen_pass_layouts[] = {
                common_dsl,
                per_model_dsl,
                framebuffer_top_dsl,
                framebuffer_bottom_dsl,
                shadow_textures_dsl,
                depth_textures_dsl,
        };

        auto fullscreen_pass_pl_create_info = vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(6)
            .setPSetLayouts(fullscreen_pass_layouts);

        layouts[pass_enum::Fullscreen] = device.createPipelineLayout(fullscreen_pass_pl_create_info);
    }


    void shader_resource_manager::create_descriptor_pool() {
        vk::DescriptorPoolCreateInfo pool_create_info = {};
        pool_create_info.maxSets = 32;  // Nova hopefully won't need too many
        pool_create_info.poolSizeCount = 2;

        // TODO: Tune these values for actual usage needs
        vk::DescriptorPoolSize sizes[] = {
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(25),
                vk::DescriptorPoolSize().setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(3),
        };

        pool_create_info.pPoolSizes = sizes;

        descriptor_pool = device.createDescriptorPool(pool_create_info);
    }

    void shader_resource_manager::create_descriptor_sets() {
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
                .setDescriptorSetCount(9)
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

        for(auto& stuff : layouts) {
            device.destroyPipelineLayout(stuff.second);
        }

        device.destroyDescriptorSetLayout(block_textures_dsl);
        device.destroyDescriptorSetLayout(custom_textures_dsl);
        device.destroyDescriptorSetLayout(shadow_textures_dsl);
        device.destroyDescriptorSetLayout(depth_textures_dsl);
        device.destroyDescriptorSetLayout(common_dsl);
        device.destroyDescriptorSetLayout(framebuffer_top_dsl);
        device.destroyDescriptorSetLayout(framebuffer_bottom_dsl);
        device.destroyDescriptorSetLayout(block_light_dsl);
        device.destroyDescriptorSetLayout(per_model_dsl);

        // Highly ineffecient but it's destruction code so idc
        device.freeDescriptorSets(descriptor_pool, 1, &block_textures);
        device.freeDescriptorSets(descriptor_pool, 1, &custom_textures);
        device.freeDescriptorSets(descriptor_pool, 1, &shadow_textures);
        device.freeDescriptorSets(descriptor_pool, 1, &depth_textures);
        device.freeDescriptorSets(descriptor_pool, 1, &common_descriptors);
        device.freeDescriptorSets(descriptor_pool, 1, &framebuffer_top);
        device.freeDescriptorSets(descriptor_pool, 1, &framebuffer_bottom);
        device.freeDescriptorSets(descriptor_pool, 1, &block_light);
        device.freeDescriptorSets(descriptor_pool, 1, &per_model_descriptors);
    }

    std::shared_ptr<shader_resource_manager> shader_resource_manager::get_instance() {
        if(!instance) {
            instance = std::make_shared<shader_resource_manager>();
        }

        return instance;
    }

    vk::PipelineLayout shader_resource_manager::get_layout_for_pass(pass_enum pass) {
        return layouts[pass];
    }
}

