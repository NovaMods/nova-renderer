#pragma once

#include <spirv_glsl.hpp>

#include "nova_renderer/rhi/resource_binder.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova {
    namespace renderer {
        struct RhiGraphicsPipelineState;
    }
} // namespace nova

namespace nova::renderer::rhi {
    class VulkanRenderDevice;

    class VulkanResourceBinder final : public RhiResourceBinder {
    public:
        VulkanResourceBinder(const RhiGraphicsPipelineState& pipeline_state, VulkanRenderDevice& device, rx::memory::allocator& allocator);

        ~VulkanResourceBinder() override = default;

        void bind_image(const rx::string& binding_name, RhiImage* image) override;

        void bind_buffer(const rx::string& binding_name, RhiBuffer* buffer) override;

        void bind_sampler(const rx::string& binding_name, RhiSampler* sampler) override;

        void bind_image_array(const rx::string& binding_name, const rx::vector<RhiImage*>& images) override;

        void bind_buffer_array(const rx::string& binding_name, const rx::vector<RhiBuffer*>& buffers) override;

        void bind_sampler_array(const rx::string& binding_name, const rx::vector<RhiSampler*>& samplers) override;

    private:
        VulkanRenderDevice* device;

        rx::memory::allocator* allocator;

        void create_descriptor_set_layouts(const rx::map<rx::string, RhiResourceBindingDescription>& all_bindings);
    };
} // namespace nova::renderer::rhi
