#pragma once

#include <rx/core/deferred_function.h>
#include <vulkan/vulkan.hpp>

#include "nova_renderer/rhi/resource_binder.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova {
    namespace renderer {
        struct RhiGraphicsPipelineState;
    }
} // namespace nova

namespace nova::renderer::rhi {
    class VulkanRenderDevice;

    /*!
     * \brief Binding resources in Vulkan!
     *
     * This is basically a thing wrapper around descriptor sets. The constructor creates the descriptor sets, the destructor destroys them
     */
    class VulkanResourceBinder final : public RhiResourceBinder {
    public:
#pragma region Lifecycle
        VulkanResourceBinder(const RhiGraphicsPipelineState& pipeline_state, VulkanRenderDevice& device, rx::memory::allocator& allocator);

        VulkanResourceBinder(const VulkanResourceBinder& other) = delete;
        VulkanResourceBinder& operator=(const VulkanResourceBinder& other) = delete;

        VulkanResourceBinder(VulkanResourceBinder&& old) noexcept = default;
        VulkanResourceBinder& operator=(VulkanResourceBinder&& old) noexcept = default;

        ~VulkanResourceBinder() override;
#pragma endregion

#pragma region RhiResourceBinder
        void bind_image(const rx::string& binding_name, RhiImage* image) override;

        void bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) override;

        void bind_sampler(const rx::string& binding_name, RhiSampler* sampler) override;

        void bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) override;

        void bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) override;

        void bind_sampler_array(const rx::string& binding_name, const rx::vector<RhiSampler*>& samplers) override;
#pragma endregion

        [[nodiscard]] vk::PipelineLayout get_layout() const;

        [[nodiscard]] const rx::vector<vk::DescriptorSet>& get_sets();

    private:
        bool dirty = false;

        VulkanRenderDevice* render_device;

        rx::memory::allocator* allocator;

        vk::AllocationCallbacks vk_allocator;

        rx::vector<vk::DescriptorSetLayout> ds_layouts;

        /*!
         * \brief Layout for pipelines that can access this binder's resources
         */
        vk::PipelineLayout layout;

        /*!
         * \brief Descriptor sets for this binder
         */
        rx::vector<vk::DescriptorSet> sets;

        rx::map<rx::string, RhiResourceBindingDescription> bindings;

        rx::map<rx::string, rx::vector<RhiImage*>> bound_images;
        rx::map<rx::string, rx::vector<RhiBuffer*>> bound_buffers;
        rx::map<rx::string, rx::vector<RhiSampler*>> bound_samplers;

        void update_all_descriptors();
    };
} // namespace nova::renderer::rhi
