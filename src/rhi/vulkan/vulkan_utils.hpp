#pragma once

#include <vulkan/vulkan.hpp>

#include "nova_renderer/renderpack_data.hpp"
#include "nova_renderer/rhi/command_list.hpp"

// idk maybe this header is included in places that already include Vulkan? Either way I want this include here and not anywhere else
// ReSharper disable once CppUnusedIncludeDirective
#include "vulkan.hpp"

namespace nova {
    namespace renderer {
        enum class BlendOp;
        enum class BlendFactor;
        enum class StencilOp;
        enum class CompareOp;
    } // namespace renderer
} // namespace nova

namespace nova::renderer::rhi {
    class VulkanRenderDevice;
    VkImageLayout to_vk_image_layout(ResourceState layout);

    VkAccessFlags to_vk_access_flags(ResourceAccess access);

    VkPrimitiveTopology to_primitive_topology(renderpack::RPPrimitiveTopology topology);

    VkBlendFactor to_blend_factor(BlendFactor factor);

    VkBlendOp to_blend_op(const BlendOp blend_op);

    VkCompareOp to_compare_op(CompareOp compare_op);

    VkStencilOp to_stencil_op(StencilOp stencil_op);

    VkFormat to_vk_format(PixelFormat format);

    VkFilter to_vk_filter(TextureFilter filter);

    VkSamplerAddressMode to_vk_address_mode(TextureCoordWrapMode wrap_mode);

    vk::DescriptorType to_vk_descriptor_type(DescriptorType type);

    vk::ShaderStageFlags to_vk_shader_stage_flags(ShaderStage flags);

    rx::string to_string(VkResult result);

    rx::string to_string(VkObjectType obj_type);

    [[nodiscard]] VkFormat to_vk_vertex_format(VertexFieldFormat field);

    [[nodiscard]] rx::vector<vk::DescriptorSetLayout> create_descriptor_set_layouts(
        const rx::map<rx::string, RhiResourceBindingDescription>& all_bindings,
        VulkanRenderDevice& render_device,
        rx::memory::allocator& allocator);;

    /*!
     * \brief Wraps a Rex allocator so the Vulkan driver can use it
     */
    inline vk::AllocationCallbacks wrap_allocator(rx::memory::allocator& allocator_in);

    bool operator&(const ShaderStage& lhs, const ShaderStage& rhs);

    RX_HINT_FORCE_INLINE vk::AllocationCallbacks wrap_allocator(rx::memory::allocator& allocator_in) {
        vk::AllocationCallbacks callbacks{};

        callbacks.pUserData = &allocator_in;
        callbacks.setPfnAllocation(
            +[](void* user_data, size_t size, size_t /* alignment */, VkSystemAllocationScope /* allocation_scope */) -> void* {
            auto allocator = reinterpret_cast<rx::memory::allocator*>(user_data);
            return allocator->allocate(size);
        });
        callbacks.pfnReallocation = +[](void* user_data,
                                       void* original,
                                       size_t size,
                                       size_t /* alignment */,
                                       VkSystemAllocationScope /* allocation_scope */) -> void* {
            auto allocator = reinterpret_cast<rx::memory::allocator*>(user_data);
            return allocator->reallocate(reinterpret_cast<rx_byte*>(original), size);
        };
        callbacks.pfnFree = +[](void* user_data, void* memory) {
            auto allocator = reinterpret_cast<rx::memory::allocator*>(user_data);
            allocator->deallocate(reinterpret_cast<rx_byte*>(memory));
        };

        return callbacks;
    }
} // namespace nova::renderer::rhi

// Only validate errors in debug mode
// Release mode needs to be fast A F
#ifdef NOVA_DEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NOVA_CHECK_RESULT(expr)                                                                                                            \
    {                                                                                                                                      \
        const VkResult result = (expr);                                                                                                    \
        if(result != VK_SUCCESS) {                                                                                                         \
            logger->error("%s:%u=>%s=%s", __FILE__, __LINE__, #expr, to_string(result));                                                   \
        }                                                                                                                                  \
    }
#else
#define NOVA_CHECK_RESULT(expr) expr
#endif
