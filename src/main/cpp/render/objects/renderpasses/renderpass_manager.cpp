/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#include <vulkan/vulkan.hpp>
#include "renderpass_manager.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"
#include <easylogging++.h>

namespace nova {
    vk::RenderPass make_render_pass(const render_pass& pass, std::shared_ptr<texture_manager> textures, std::shared_ptr<render_context> context);

    std::unordered_map<std::string, vk::RenderPass> make_passes(const shaderpack_data& data, std::shared_ptr<texture_manager> textures,
                                                                std::shared_ptr<render_context> context) {
        std::unordered_map<std::string, vk::RenderPass> renderpasses;

        for(const auto& named_pass : data.passes) {
            renderpasses[named_pass.first] = make_render_pass(named_pass.second, textures, context);
        }

        return renderpasses;
    }

    vk::RenderPass make_render_pass(const render_pass& pass, std::shared_ptr<texture_manager> textures, std::shared_ptr<render_context> context) {

        std::vector<vk::AttachmentDescription> attachments;

        std::vector<vk::AttachmentReference> color_refs;

        if(pass.texture_outputs) {
            auto num_supported_attachments = context->gpu.props.limits.maxColorAttachments;
            const auto& texture_outputs_vec = pass.texture_outputs.value();
            if(texture_outputs_vec.size() > num_supported_attachments) {
                LOG(ERROR) << "You're trying to use " << texture_outputs_vec.size() << " color attachments with pass "
                           << pass.name << ", but your GPU only supports " << num_supported_attachments;

            } else {
                for(const auto &color_attachment_name : texture_outputs_vec) {
                    const auto &texture = textures->get_texture(color_attachment_name);

                    vk::AttachmentDescription color_attachment = {};
                    color_attachment.format = texture.get_format();
                    color_attachment.samples = vk::SampleCountFlagBits::e1;
                    color_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
                    color_attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
                    color_attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

                    attachments.push_back(color_attachment);
                }
            }
        }

        if(pass.texture_inputs) {
            vk::AttachmentReference ref = {};
            ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
            ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
            color_refs.push_back(ref);
        }

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass.pColorAttachments = color_refs.data();

        vk::RenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        final_pass = context->device.createRenderPass(render_pass_create_info);
    }


    void renderpass_manager::create_final_renderpass(const vk::Extent2D& window_size) {
        std::vector<vk::AttachmentDescription> attachments;

        std::vector<vk::AttachmentReference> color_refs;

        // Just one color buffer in the final pass
        vk::AttachmentDescription color_attachment = {};
        color_attachment.format = context->swapchain_format;
        color_attachment.samples = vk::SampleCountFlagBits::e1;
        color_attachment.loadOp = vk::AttachmentLoadOp::eDontCare;
        color_attachment.initialLayout = vk::ImageLayout::eUndefined;
        color_attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        attachments.push_back(color_attachment);

        vk::AttachmentReference ref = {};
        ref.attachment = static_cast<uint32_t>(attachments.size() - 1);
        ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
        color_refs.push_back(ref);

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass.pColorAttachments = color_refs.data();

        vk::RenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        final_pass = context->device.createRenderPass(render_pass_create_info);

        LOG(TRACE) << "Created final renderpass";
    }

    void renderpass_manager::create_main_renderpass(const vk::Extent2D& window_size) {
        /*main_pass = renderpass_builder()
                .set_framebuffer_size(window_size.width, window_size.height)
                .add_color_buffer()
                .build();

        LOG(TRACE) << "Created main renderpass";*/
    }

    const vk::RenderPass renderpass_manager::get_main_renderpass() const {
        return main_pass;
    }

    const vk::RenderPass renderpass_manager::get_final_renderpass() const {
        return final_pass;
    }

    void renderpass_manager::rebuild_all(const vk::Extent2D& main_shadow_size, const vk::Extent2D& light_shadow_size, const vk::Extent2D& window_size) {
        create_main_renderpass(window_size);
        create_final_renderpass(window_size);

        create_final_framebuffers(window_size);
    }

    void renderpass_manager::create_final_framebuffers(const vk::Extent2D &window_size) {
        final_framebuffers.reserve(context->swapchain_image_views.size());

        for(size_t i = 0; i < context->swapchain_image_views.size(); i++) {
            vk::ImageView attachments[] = {
                    context->swapchain_image_views[i]
            };

            vk::FramebufferCreateInfo framebuffer_create_info = {};
            framebuffer_create_info.renderPass = final_pass;
            framebuffer_create_info.attachmentCount = 1;
            framebuffer_create_info.pAttachments = attachments;
            framebuffer_create_info.width = window_size.width;
            framebuffer_create_info.height = window_size.height;
            framebuffer_create_info.layers = 1;

            final_framebuffers.push_back(context->device.createFramebuffer(framebuffer_create_info));
        }
    }

    const vk::Framebuffer renderpass_manager::get_framebuffer(uint32_t framebuffer_idx) const {
        return final_framebuffers[framebuffer_idx];
    }

    renderpass_manager::~renderpass_manager() {
        auto device = context->device;

        device.destroyRenderPass(main_shadow_pass);
        device.destroyRenderPass(light_shadow_pass);
        device.destroyRenderPass(main_pass);
        device.destroyRenderPass(final_pass);

        for(auto& fb : final_framebuffers) {
            device.destroyFramebuffer(fb);
        }
    }
}
