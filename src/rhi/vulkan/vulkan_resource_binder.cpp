#include "vulkan_resource_binder.hpp"

#include <rx/core/algorithm/max.h>
#include <rx/core/log.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

#include "../../renderer/pipeline_reflection.hpp"
#include "vulkan_render_device.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VulkanResourceBinder", logger);

    VulkanResourceBinder::VulkanResourceBinder(const RhiGraphicsPipelineState& pipeline_state,
                                               VulkanRenderDevice& device,
                                               rx::memory::allocator& allocator)
        : device{&device}, allocator{&allocator} {
        rx::map<rx::string, RhiResourceBindingDescription> bindings;

        get_shader_module_descriptors(pipeline_state.vertex_shader.source, ShaderStage::Vertex, bindings);

        if(pipeline_state.geometry_shader) {
            get_shader_module_descriptors(pipeline_state.geometry_shader->source, ShaderStage::Geometry, bindings);
        }
        if(pipeline_state.pixel_shader) {
            get_shader_module_descriptors(pipeline_state.pixel_shader->source, ShaderStage::Fragment, bindings);
        }

        create_descriptor_set_layouts(bindings);
    }
    void VulkanResourceBinder::create_descriptor_set_layouts(const rx::map<rx::string, RhiResourceBindingDescription>& all_bindings) {
        const auto max_sets = device->gpu.props.limits.maxBoundDescriptorSets;

        uint32_t num_sets = 0;
        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            if(desc.set >= max_sets) {
                logger->error("Descriptor set %u is out of range - your GPU only supports %u sets!", desc.set, max_sets);
            } else {
                num_sets = rx::algorithm::max(num_sets, desc.set + 1);
            }
        });

        variable_descriptor_counts.resize(num_sets, 0);

        // Some precalculations so we know how much room we actually need
        rx::vector<uint32_t> num_bindings_per_set{allocator};
        num_bindings_per_set.resize(num_sets);

        all_bindings.each_value([&](const RhiResourceBindingDescription& desc) {
            num_bindings_per_set[desc.set] = rx::algorithm::max(num_bindings_per_set[desc.set], desc.binding + 1);
        });

        rx::vector<rx::vector<VkDescriptorSetLayoutBinding>> bindings_by_set{allocator};
        rx::vector<rx::vector<VkDescriptorBindingFlags>> binding_flags_by_set{allocator};
        bindings_by_set.reserve(num_sets);
        binding_flags_by_set.reserve(num_sets);

        uint32_t set = 0;

        num_bindings_per_set.each_fwd([&](const uint32_t num_bindings) {
            // Emplace back vectors large enough to hold all the bindings we have
            bindings_by_set.emplace_back(num_bindings);
            binding_flags_by_set.emplace_back(num_bindings);

            logger(rx::log::level::k_verbose, "Set %u has %u bindings", set, num_bindings);
            set++;
        });

        all_bindings.each_value([&](const RhiResourceBindingDescription& binding) {
            if(binding.set >= bindings_by_set.size()) {
                logger(rx::log::level::k_error, "You've skipped one or more descriptor sets! Don't do that, Nova can't handle it");
                return true;
            }

            VkDescriptorSetLayoutBinding descriptor_binding = {};
            descriptor_binding.binding = binding.binding;
            descriptor_binding.descriptorType = to_vk_descriptor_type(binding.type);
            descriptor_binding.descriptorCount = binding.count;
            descriptor_binding.stageFlags = to_vk_shader_stage_flags(binding.stages);

            logger(rx::log::level::k_verbose,
                   "Descriptor %u.%u is type %s",
                   binding.set,
                   binding.binding,
                   descriptor_type_to_string(binding.type));

            if(binding.is_unbounded) {
                binding_flags_by_set[binding.set][binding.binding] = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
                                                                     VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

                // Record the maximum number of descriptors in the variable size array in this set
                variable_descriptor_counts[binding.set] = binding.count;

                logger->verbose("Descriptor %u.%u is unbounded", binding.set, binding.binding);

            } else {
                binding_flags_by_set[binding.set][binding.binding] = 0;
            }

            bindings_by_set[binding.set][binding.binding] = descriptor_binding;

            return true;
        });

        rx::vector<VkDescriptorSetLayoutCreateInfo> dsl_create_infos{allocator};
        dsl_create_infos.reserve(bindings_by_set.size());

        rx::vector<VkDescriptorSetLayoutBindingFlagsCreateInfo> flag_infos{allocator};
        flag_infos.reserve(bindings_by_set.size());

        // We may make bindings_by_set much larger than it needs to be is there's multiple descriptor bindings per set. Thus, only iterate
        // through the sets we actually care about
        bindings_by_set.each_fwd([&](const rx::vector<VkDescriptorSetLayoutBinding>& bindings) {
            VkDescriptorSetLayoutCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            create_info.bindingCount = static_cast<uint32_t>(bindings.size());
            create_info.pBindings = bindings.data();

            const auto& flags = binding_flags_by_set[dsl_create_infos.size()];
            VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags = {};
            binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            binding_flags.bindingCount = static_cast<uint32_t>(flags.size());
            binding_flags.pBindingFlags = flags.data();
            flag_infos.emplace_back(binding_flags);

            create_info.pNext = &flag_infos[flag_infos.size() - 1];

            dsl_create_infos.push_back(create_info);
        });

        rx::vector<VkDescriptorSetLayout> layouts{allocator};
        auto vk_alloc = wrap_allocator(allocator);
        layouts.resize(dsl_create_infos.size());
        for(size_t i = 0; i < dsl_create_infos.size(); i++) {
            vkCreateDescriptorSetLayout(device, &dsl_create_infos[i], &vk_alloc, &layouts[i]);
        }

        return layouts;
    }

} // namespace nova::renderer::rhi
