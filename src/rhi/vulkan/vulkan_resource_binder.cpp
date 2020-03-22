#include "vulkan_resource_binder.hpp"

#include <rx/core/algorithm/max.h>
#include <rx/core/log.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

#include "../../renderer/pipeline_reflection.hpp"
#include "vulkan_render_device.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VulkanResourceBinder", logger);

    template <typename ResourceType>
    void bind_resource_array(const rx::string& binding_name,
                             const rx::vector<ResourceType*>& resources,
                             rx::map<rx::string, rx::vector<ResourceType*>>& bound_resources);

    VulkanResourceBinder::VulkanResourceBinder(const RhiGraphicsPipelineState& pipeline_state,
                                               VulkanRenderDevice& device,
                                               rx::memory::allocator& allocator)
        : render_device{&device},
          allocator{&allocator},
          vk_allocator{wrap_allocator(allocator)},
          ds_layouts{&allocator},
          bindings{&allocator},
          bound_images{&allocator},
          bound_buffers{&allocator},
          bound_samplers{&allocator} {

        get_shader_module_descriptors(pipeline_state.vertex_shader.source, ShaderStage::Vertex, bindings);

        if(pipeline_state.geometry_shader) {
            get_shader_module_descriptors(pipeline_state.geometry_shader->source, ShaderStage::Geometry, bindings);
        }
        if(pipeline_state.pixel_shader) {
            get_shader_module_descriptors(pipeline_state.pixel_shader->source, ShaderStage::Fragment, bindings);
        }

        create_descriptor_set_layouts(bindings);

        const auto pipeline_layout_create = vk::PipelineLayoutCreateInfo()
                                                .setSetLayoutCount(ds_layouts.size())
                                                .setPSetLayouts(ds_layouts.data)
                                                .setPushConstantRangeCount(0)
                                                .setPPushConstantRanges(nullptr);

        device.device.createPipelineLayout(&pipeline_layout_create, &vk_allocator, &layout);
    }

    VulkanResourceBinder::~VulkanResourceBinder() {
        auto& device = render_device->device;

        device.destroyPipelineLayout(layout, &vk_allocator);

        ds_layouts.each_fwd([&](const vk::DescriptorSetLayout dsl) { device.destroyDescriptorSetLayout(dsl, &vk_allocator); });
    }

    void VulkanResourceBinder::bind_image(const rx::string& binding_name, RhiImage* image) {
        bind_image_array(binding_name, rx::array{allocator, image});
    }

    void VulkanResourceBinder::bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) {
        bind_buffer_array(binding_name, rx::array{allocator, buffer});
    }

    void VulkanResourceBinder::bind_sampler(const rx::string& binding_name, RhiSampler* sampler) {
        bind_sampler_array(binding_name, rx::array{allocator, sampler});
    }

    void VulkanResourceBinder::bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) {
        bind_resource_array(binding_name, images, bound_images);

        dirty = true;
    }

    void VulkanResourceBinder::bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) {
#if NOVA_DEBUG
        buffers.each_fwd([&](const RhiBuffer* buffer) {
            if(buffer->size > render_device->gpu.props.limits.maxUniformBufferRange) {
                logger->error("Cannot bind a buffer with a size greater than %u", render_device->gpu.props.limits.maxUniformBufferRange);
            }
        });
#endif

        bind_resource_array(binding_name, buffers, bound_buffers);

        dirty = true;
    }

    void VulkanResourceBinder::bind_sampler_array(const rx::string& binding_name, const rx::vector<RhiSampler*>& samplers) {
        bind_resource_array(binding_name, samplers, bound_samplers);

        dirty = true;
    }

    vk::PipelineLayout VulkanResourceBinder::get_layout() const { return layout; }

    const rx::vector<vk::DescriptorSet>& VulkanResourceBinder::get_sets() {
        if(dirty) {
            update_all_descriptors();
        }

        return sets;
    }

    void VulkanResourceBinder::create_descriptor_set_layouts(const rx::map<rx::string, RhiResourceBindingDescription>& all_bindings) {
        const auto max_sets = render_device->gpu.props.limits.maxBoundDescriptorSets;

        uint32_t num_sets = 0;
        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            if(desc.set >= max_sets) {
                logger->error("Descriptor set %u is out of range - your GPU only supports %u sets!", desc.set, max_sets);
            } else {
                num_sets = rx::algorithm::max(num_sets, desc.set + 1);
            }
        });

        rx::vector<uint32_t> variable_descriptor_counts{allocator};
        variable_descriptor_counts.resize(num_sets, 0);

        // Some precalculations so we know how much room we actually need
        rx::vector<uint32_t> num_bindings_per_set{allocator};
        num_bindings_per_set.resize(num_sets);

        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            num_bindings_per_set[desc.set] = rx::algorithm::max(num_bindings_per_set[desc.set], desc.binding + 1);
        });

        rx::vector<rx::vector<vk::DescriptorSetLayoutBinding>> bindings_by_set{allocator};
        rx::vector<rx::vector<vk::DescriptorBindingFlags>> binding_flags_by_set{allocator};
        bindings_by_set.reserve(num_sets);
        binding_flags_by_set.reserve(num_sets);

        uint32_t set = 0;

        num_bindings_per_set.each_fwd([&](const uint32_t num_bindings) {
            // Emplace back vectors large enough to hold all the bindings we have
            bindings_by_set.emplace_back(allocator, num_bindings);
            binding_flags_by_set.emplace_back(allocator, num_bindings);

            logger->verbose("Set %u has %u bindings", set, num_bindings);
            set++;
        });

        all_bindings.each_value([&](const RhiResourceBindingDescription& binding) {
            if(binding.set >= bindings_by_set.size()) {
                logger->error("You've skipped one or more descriptor sets! Don't do that, Nova can't handle it");
                return true;
            }

            const auto descriptor_binding = vk::DescriptorSetLayoutBinding()
                                                .setBinding(binding.binding)
                                                .setDescriptorType(to_vk_descriptor_type(binding.type))
                                                .setDescriptorCount(binding.count)
                                                .setStageFlags(to_vk_shader_stage_flags(binding.stages));

            logger->verbose("Descriptor %u.%u is type %s", binding.set, binding.binding, descriptor_type_to_string(binding.type));

            if(binding.is_unbounded) {
                binding_flags_by_set[binding.set][binding.binding] = vk::DescriptorBindingFlagBits::eVariableDescriptorCount |
                                                                     vk::DescriptorBindingFlagBits::ePartiallyBound;

                // Record the maximum number of descriptors in the variable size array in this set
                variable_descriptor_counts[binding.set] = binding.count;

                logger->verbose("Descriptor %u.%u is unbounded", binding.set, binding.binding);

            } else {
                binding_flags_by_set[binding.set][binding.binding] = {};
            }

            bindings_by_set[binding.set][binding.binding] = descriptor_binding;

            return true;
        });

        rx::vector<vk::DescriptorSetLayoutCreateInfo> dsl_create_infos{allocator};
        dsl_create_infos.reserve(bindings_by_set.size());

        rx::vector<vk::DescriptorSetLayoutBindingFlagsCreateInfo> flag_infos{allocator};
        flag_infos.reserve(bindings_by_set.size());

        // We may make bindings_by_set much larger than it needs to be is there's multiple descriptor bindings per set. Thus, only iterate
        // through the sets we actually care about
        bindings_by_set.each_fwd([&](const rx::vector<vk::DescriptorSetLayoutBinding>& bindings) {
            vk::DescriptorSetLayoutCreateInfo create_info = {};
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            const auto& flags = binding_flags_by_set[dsl_create_infos.size()];
            vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {};
            binding_flags.bindingCount = static_cast<uint32_t>(flags.size());
            binding_flags.pBindingFlags = flags.data();
            flag_infos.emplace_back(binding_flags);

            create_info.pNext = &flag_infos[flag_infos.size() - 1];

            dsl_create_infos.push_back(create_info);
        });

        ds_layouts.resize(dsl_create_infos.size());
        for(size_t i = 0; i < dsl_create_infos.size(); i++) {
            render_device->device.createDescriptorSetLayout(&dsl_create_infos[i], &vk_allocator, &ds_layouts[i]);
        }
    }

    void VulkanResourceBinder::update_all_descriptors() {
        rx::vector<vk::WriteDescriptorSet> writes{allocator};
        writes.reserve(bound_images.size() + bound_samplers.size() + bound_buffers.size());

        rx::vector<rx::vector<vk::DescriptorImageInfo>> all_image_infos{allocator};
        all_image_infos.reserve(bound_images.size() + bound_samplers.size());

        rx::vector<vk::DescriptorBufferInfo> all_buffer_infos{allocator};
        all_buffer_infos.reserve(bound_buffers.size());

        bound_images.each_pair([&](const rx::string& name, const rx::vector<RhiImage*>& images) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            rx::vector<vk::DescriptorImageInfo> image_infos{allocator};
            image_infos.reserve(images.size());

            images.each_fwd([&](const RhiImage* image) {
                const auto* vk_image = static_cast<const VulkanImage*>(image);
                auto image_info = vk::DescriptorImageInfo()
                                      .setImageView(vk_image->image_view)
                                      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
                image_infos.push_back(rx::utility::move(image_info));
            });

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(image_infos.size()))
                             .setDescriptorType(vk::DescriptorType::eSampledImage)
                             .setPImageInfo(image_infos.data());
            writes.push_back(rx::utility::move(write));
        });

        bound_samplers.each_pair([&](const rx::string& name, const rx::vector<RhiSampler*>& samplers) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            rx::vector<vk::DescriptorImageInfo> sampler_infos{allocator};
            sampler_infos.reserve(samplers.size());

            samplers.each_fwd([&](const RhiSampler* sampler) {
                const auto* vk_sampler = static_cast<const VulkanSampler*>(sampler);
                auto sampler_info = vk::DescriptorImageInfo().setSampler(vk_sampler->sampler);
                sampler_infos.push_back(rx::utility::move(sampler_info));
            });

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(sampler_infos.size()))
                             .setDescriptorType(vk::DescriptorType::eSampler)
                             .setPImageInfo(sampler_infos.data());
            writes.push_back(rx::utility::move(write));
        });

        bound_buffers.each_pair([&](const rx::string& name, const rx::vector<RhiBuffer*>& buffers) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            rx::vector<vk::DescriptorBufferInfo> buffer_infos{allocator};
            buffer_infos.reserve(buffers.size());

            buffers.each_fwd([&](const RhiBuffer* buffer) {
                const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffer);
                auto buffer_info = vk::DescriptorBufferInfo().setBuffer(vk_buffer->buffer).setOffset(0).setRange(vk_buffer->size.b_count());
                buffer_infos.push_back(rx::utility::move(buffer_info));
            });

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(buffer_infos.size()))
                             .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                             .setPBufferInfo(buffer_infos.data());
            writes.push_back(rx::utility::move(write));
        });

        render_device->device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    template <typename ResourceType>
    void bind_resource_array(const rx::string& binding_name,
                             const rx::vector<ResourceType*>& resources,
                             rx::map<rx::string, rx::vector<ResourceType*>>& bound_resources) {
        if(auto* bound_data = bound_resources.find(binding_name)) {
            *bound_data = resources;

        } else {
            bound_resources.insert(binding_name, resources);
        }
    }
} // namespace nova::renderer::rhi
