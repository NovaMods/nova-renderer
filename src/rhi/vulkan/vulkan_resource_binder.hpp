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
        VulkanResourceBinder(VulkanRenderDevice& device,
                             std::unordered_map<std::string, RhiResourceBindingDescription> bindings,
                             std::vector<vk::DescriptorSet> sets,
                             vk::PipelineLayout layout,
                             rx::memory::allocator& allocator);

        VulkanResourceBinder(const VulkanResourceBinder& other) = delete;
        VulkanResourceBinder& operator=(const VulkanResourceBinder& other) = delete;

        VulkanResourceBinder(VulkanResourceBinder&& old) noexcept = default;
        VulkanResourceBinder& operator=(VulkanResourceBinder&& old) noexcept = default;

        ~VulkanResourceBinder() override = default;
#pragma endregion

#pragma region RhiResourceBinder
        void bind_image(const std::string& binding_name, RhiImage* image) override;

        void bind_buffer(const std::string& binding_name, RhiBuffer* buffer) override;

        void bind_sampler(const std::string& binding_name, RhiSampler* sampler) override;

        void bind_image_array(const std::string& binding_name, const std::vector<RhiImage*>& images) override;

        void bind_buffer_array(const std::string& binding_name, const std::vector<RhiBuffer*>& buffers) override;

        void bind_sampler_array(const std::string& binding_name, const std::vector<RhiSampler*>& samplers) override;
#pragma endregion

        [[nodiscard]] vk::PipelineLayout get_layout() const;

        [[nodiscard]] const std::vector<vk::DescriptorSet>& get_sets();

    private:
        bool dirty = false;

        VulkanRenderDevice* render_device;

        rx::memory::allocator* allocator;

        /*!
         * \brief Layout for pipelines that can access this binder's resources
         */
        vk::PipelineLayout layout;

        /*!
         * \brief Descriptor sets for this binder
         */
        std::vector<vk::DescriptorSet> sets;

        std::unordered_map<std::string, RhiResourceBindingDescription> bindings;

        std::unordered_map<std::string, std::vector<RhiImage*>> bound_images;
        std::unordered_map<std::string, std::vector<RhiBuffer*>> bound_buffers;
        std::unordered_map<std::string, std::vector<RhiSampler*>> bound_samplers;

        void update_all_descriptors();
    };
} // namespace nova::renderer::rhi
