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

    vk::Framebuffer make_framebuffer(const render_pass &pass, const vk::RenderPass renderpass, std::shared_ptr<texture_manager> textures, std::shared_ptr<render_context> context);

    std::unordered_map<std::string, std::pair<vk::RenderPass, vk::Framebuffer>> make_passes(const shaderpack_data& data, std::shared_ptr<texture_manager> textures,
                                                                                            std::shared_ptr<render_context> context) {
        std::unordered_map<std::string, std::pair<vk::RenderPass, vk::Framebuffer>> renderpasses;

        for(const auto& named_pass : data.passes) {
            const auto renderpass = make_render_pass(named_pass.second, textures, context);
            renderpasses[named_pass.first].first = renderpass;
            renderpasses[named_pass.first].second = make_framebuffer(named_pass.second, renderpass, textures, context);
        }

        return renderpasses;
    }

    vk::RenderPass make_render_pass(const render_pass& pass, std::shared_ptr<texture_manager> textures, std::shared_ptr<render_context> context) {

        std::vector<vk::AttachmentDescription> attachments;

        std::vector<vk::AttachmentReference> color_refs;
        optional<vk::AttachmentReference> depth_ref;

        if(pass.texture_outputs) {
            auto num_supported_attachments = context->gpu.props.limits.maxColorAttachments;
            const auto& texture_outputs_vec = pass.texture_outputs.value();

            // The size and name of the attachment at index n - 1, allowing us to validate that all the textures in a
            // given pass are the same size
            vk::Extent2D last_texture_size;
            std::string last_texture_name;

            if(texture_outputs_vec.size() > num_supported_attachments) {
                LOG(ERROR) << "You're trying to use " << texture_outputs_vec.size() << " color attachments with pass "
                           << pass.name << ", but your GPU only supports " << num_supported_attachments;

            } else {
                const auto& first_tex = textures->get_texture(texture_outputs_vec[0]);
                last_texture_size = first_tex.get_size();
                last_texture_name = first_tex.get_name();

                for(const auto &color_attachment_name : texture_outputs_vec) {
                    const auto &texture = textures->get_texture(color_attachment_name);

                    if(texture.get_size() != last_texture_size) {
                        LOG(ERROR) << "Texture " << texture.get_name() << " does not have the same size as texture "
                                   << last_texture_name << ". In order to use two textures in the same pass, they must have the same size";
                    }

                    last_texture_size = texture.get_size();
                    last_texture_name = texture.get_name();

                    vk::AttachmentDescription color_attachment = {};
                    color_attachment.format = texture.get_format();
                    color_attachment.samples = vk::SampleCountFlagBits::e1;
                    color_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
                    color_attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
                    color_attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
                    color_attachment.storeOp = vk::AttachmentStoreOp::eStore;

                    auto color_ref = vk::AttachmentReference()
                        .setAttachment(static_cast<uint32_t>(attachments.size()))
                        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

                    color_refs.push_back(color_ref);
                    attachments.push_back(color_attachment);
                }
            }
        }

        if(pass.depth_output) {
            const auto& depth_tex = textures->get_texture(pass.depth_output.value());

            auto depth_attachment = vk::AttachmentDescription()
                .setFormat(depth_tex.get_format())
                .setSamples(vk::SampleCountFlagBits::e1)
                .setLoadOp(vk::AttachmentLoadOp::eLoad)
                .setInitialLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore);

            depth_ref = vk::AttachmentReference{};
            depth_ref->setAttachment(static_cast<uint32_t>(attachments.size()));
            depth_ref->setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

            attachments.push_back(depth_attachment);
        }

        vk::SubpassDescription subpass = {};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass.pColorAttachments = color_refs.data();
        if(depth_ref) {
            subpass.setPDepthStencilAttachment(&depth_ref.value());
        }

        vk::RenderPassCreateInfo render_pass_create_info = {};
        render_pass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        render_pass_create_info.pAttachments = attachments.data();
        render_pass_create_info.subpassCount = 1;
        render_pass_create_info.pSubpasses = &subpass;
        render_pass_create_info.dependencyCount = 0;

        return context->device.createRenderPass(render_pass_create_info);
    }


    vk::Framebuffer make_framebuffer(const render_pass &pass, const vk::RenderPass renderpass, std::shared_ptr<texture_manager> textures,
                                     std::shared_ptr<render_context> context) {
        std::vector<vk::ImageView> attachments;
        vk::Extent2D framebuffer_size;
        if(pass.texture_outputs) {
            const auto& outputs = pass.texture_outputs.value();

            for(const auto& output_name : outputs) {
                const auto& tex = textures->get_texture(output_name);
                attachments.push_back(tex.get_image_view());
                framebuffer_size = tex.get_size();
            }
        }
        if(pass.depth_output) {
            const auto& depth_tex_name = pass.depth_output.value();
            const auto& depth_tex = textures->get_texture(depth_tex_name);
            attachments.push_back(depth_tex.get_image_view());
            framebuffer_size = depth_tex.get_size();
        }

        if(!attachments.empty()) {
            auto framebuffer_create_info = vk::FramebufferCreateInfo()
                    .setAttachmentCount(attachments.size())
                    .setPAttachments(attachments.data())
                    .setRenderPass(renderpass)
                    .setWidth(framebuffer_size.width)
                    .setHeight(framebuffer_size.height)
                    .setLayers(1);

            return context->device.createFramebuffer(framebuffer_create_info);
        }

        LOG(ERROR) << "No framebuffer attachments for pass " << pass.name << ". This is an error and you should fix it";

        return vk::Framebuffer();
    }
}
