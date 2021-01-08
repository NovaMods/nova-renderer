#include "vulkan_resource_binder.hpp"

#include <rx/core/algorithm/max.h>
#include <rx/core/log.h>

#include "nova_renderer/rhi/rhi_types.hpp"

#include "../../renderer/pipeline_reflection.hpp"
#include "vulkan_render_device.hpp"
#include "vulkan_utils.hpp"

namespace nova::renderer::rhi {
    RX_LOG("VulkanResourceBinder", logger);

    template <typename ResourceType>
    void bind_resource_array(const std::string& binding_name,
                             const std::vector<ResourceType*>& resources,
                             std::unordered_map<std::string, std::vector<ResourceType*>>& bound_resources);

    VulkanResourceBinder::VulkanResourceBinder(VulkanRenderDevice& device,
                                               std::unordered_map<std::string, RhiResourceBindingDescription> bindings,
                                               std::vector<vk::DescriptorSet> sets,
                                               vk::PipelineLayout layout,
                                               rx::memory::allocator& allocator)
        : render_device{&device},
          allocator{&allocator},
          layout{layout},
          sets{std::move(sets)},
          bindings{std::move(bindings)},
          bound_images{&allocator},
          bound_buffers{&allocator},
          bound_samplers{&allocator} {}

    void VulkanResourceBinder::bind_image(const std::string& binding_name, RhiImage* image) {
        bind_image_array(binding_name, {allocator, std::array{image}});
    }

    void VulkanResourceBinder::bind_buffer(const std::string& binding_name, RhiBuffer* buffer) {
        bind_buffer_array(binding_name, {allocator, std::array{buffer}});
    }

    void VulkanResourceBinder::bind_sampler(const std::string& binding_name, RhiSampler* sampler) {
        bind_sampler_array(binding_name, {allocator, std::array{sampler}});
    }

    void VulkanResourceBinder::bind_image_array(const std::string& binding_name, const std::vector<RhiImage*>& images) {
        bind_resource_array(binding_name, images, bound_images);

        dirty = true;
    }

    void VulkanResourceBinder::bind_buffer_array(const std::string& binding_name, const std::vector<RhiBuffer*>& buffers) {
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

    void VulkanResourceBinder::bind_sampler_array(const std::string& binding_name, const std::vector<RhiSampler*>& samplers) {
        bind_resource_array(binding_name, samplers, bound_samplers);

        dirty = true;
    }

    vk::PipelineLayout VulkanResourceBinder::get_layout() const { return layout; }

    const std::vector<vk::DescriptorSet>& VulkanResourceBinder::get_sets() {
        if(dirty) {
            update_all_descriptors();
        }

        return sets;
    }

    void VulkanResourceBinder::update_all_descriptors() {
        std::vector<vk::WriteDescriptorSet> writes{allocator};
        writes.reserve(bound_images.size() + bound_samplers.size() + bound_buffers.size());

        std::vector<std::vector<vk::DescriptorImageInfo>> all_image_infos{allocator};
        all_image_infos.reserve(bound_images.size() + bound_samplers.size());

        std::vector < std::vector<vk::DescriptorBufferInfo>> all_buffer_infos{allocator};
        all_buffer_infos.reserve(bound_buffers.size());

        bound_images.each_pair([&](const std::string& name, const std::vector<RhiImage*>& images) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            std::vector<vk::DescriptorImageInfo> image_infos{allocator};
            image_infos.reserve(images.size());

            images.each_fwd([&](const RhiImage* image) {
                const auto* vk_image = static_cast<const VulkanImage*>(image);
                auto image_info = vk::DescriptorImageInfo()
                                      .setImageView(vk_image->image_view)
                                      .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
                image_infos.push_back(std::move(image_info));
            });

            all_image_infos.push_back(std::move(image_infos));

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(images.size()))
                             .setDescriptorType(vk::DescriptorType::eSampledImage)
                             .setPImageInfo(all_image_infos.last().data());
            writes.push_back(std::move(write));
        });

        bound_samplers.each_pair([&](const std::string& name, const std::vector<RhiSampler*>& samplers) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            std::vector<vk::DescriptorImageInfo> sampler_infos{allocator};
            sampler_infos.reserve(samplers.size());

            samplers.each_fwd([&](const RhiSampler* sampler) {
                const auto* vk_sampler = static_cast<const VulkanSampler*>(sampler);
                auto sampler_info = vk::DescriptorImageInfo().setSampler(vk_sampler->sampler);
                sampler_infos.push_back(std::move(sampler_info));
            });

            all_image_infos.push_back(std::move(sampler_infos));

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(samplers.size()))
                             .setDescriptorType(vk::DescriptorType::eSampler)
                             .setPImageInfo(all_image_infos.last().data());
            writes.push_back(std::move(write));
        });

        bound_buffers.each_pair([&](const std::string& name, const std::vector<RhiBuffer*>& buffers) {
            const auto& binding = *bindings.find(name);
            const auto set = sets[binding.set];

            std::vector<vk::DescriptorBufferInfo> buffer_infos{allocator};
            buffer_infos.reserve(buffers.size());

            buffers.each_fwd([&](const RhiBuffer* buffer) {
                const auto* vk_buffer = static_cast<const VulkanBuffer*>(buffer);
                auto buffer_info = vk::DescriptorBufferInfo().setBuffer(vk_buffer->buffer).setOffset(0).setRange(vk_buffer->size.b_count());
                buffer_infos.push_back(std::move(buffer_info));
            });

            all_buffer_infos.push_back(std::move(buffer_infos));

            auto write = vk::WriteDescriptorSet()
                             .setDstSet(set)
                             .setDstBinding(binding.binding)
                             .setDstArrayElement(0)
                             .setDescriptorCount(static_cast<uint32_t>(buffers.size()))
                             .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                             .setPBufferInfo(all_buffer_infos.last().data());
            writes.push_back(std::move(write));
        });

        render_device->device.updateDescriptorSets(static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }

    template <typename ResourceType>
    void bind_resource_array(const std::string& binding_name,
                             const std::vector<ResourceType*>& resources,
                             std::unordered_map<std::string, std::vector<ResourceType*>>& bound_resources) {
        if(auto* bound_data = bound_resources.find(binding_name)) {
            *bound_data = resources;

        } else {
            bound_resources.insert(binding_name, resources);
        }
    }
} // namespace nova::renderer::rhi
