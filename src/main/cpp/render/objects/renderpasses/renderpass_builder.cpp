/*!
 * \author ddubois 
 * \date 21-Oct-17.
 */

#include <vulkan/vulkan.hpp>
#include "renderpass_builder.h"
#include "../../vulkan/render_context.h"
#include "../../nova_renderer.h"
#include <easylogging++.h>

namespace nova {
    vk::RenderPass make_render_pass(const render_pass& pass, texture_manager& textures, std::shared_ptr<render_context> context, std::shared_ptr<swapchain_manager> swapchain);

    std::tuple<vk::Framebuffer, vk::Extent2D, int32_t> make_framebuffer(const render_pass &pass, const vk::RenderPass renderpass, texture_manager& textures, std::shared_ptr<render_context> context);

    std::unordered_map<std::string, pass_vulkan_information> make_passes(const shaderpack_data& data, texture_manager& textures,
                                                                                            std::shared_ptr<render_context> context, std::shared_ptr<swapchain_manager> swapchain) {
        std::unordered_map<std::string, pass_vulkan_information> renderpasses;

        for(const auto& named_pass : data.passes) {
            auto pass_vk_info = pass_vulkan_information{};

            const auto renderpass = make_render_pass(named_pass.second, textures, context, swapchain);
            pass_vk_info.renderpass = renderpass;

            auto[framebuffer, size, depth_idx] = make_framebuffer(named_pass.second, renderpass, textures, context);
            pass_vk_info.frameBuffer = framebuffer;
            pass_vk_info.depth_idx = depth_idx;
            if(size.width > 0 && size.height > 0) {
                pass_vk_info.framebuffer_size = size;

            } else {
                pass_vk_info.framebuffer_size = swapchain->get_swapchain_extent();
            }

            pass_vk_info.num_attachments = static_cast<uint32_t>(named_pass.second.texture_outputs.value_or(std::vector<texture_attachment>{}).size());

            if(named_pass.second.texture_outputs) {
                pass_vk_info.texture_outputs = named_pass.second.texture_outputs.value();
            }

            renderpasses[named_pass.first] = pass_vk_info;
        }

        return renderpasses;
    }

    vk::RenderPass make_render_pass(const render_pass& pass, texture_manager& textures, std::shared_ptr<render_context> context, std::shared_ptr<swapchain_manager> swapchain) {
        LOG(INFO) << "Making VkRenderPass for pass " << pass.name;

        std::vector<vk::AttachmentDescription> attachments;

        std::vector<vk::AttachmentReference> color_refs;
        optional<vk::AttachmentReference> depth_ref;

        if(pass.texture_outputs) {
            auto num_supported_attachments = context->gpu.props.limits.maxColorAttachments;
            const auto &texture_outputs_vec = pass.texture_outputs.value();

            // The size and name of the attachment at index n - 1, allowing us to validate that all the textures in a
            // given pass are the same size
            vk::Extent2D last_texture_size;
            std::string last_texture_name;

            if(texture_outputs_vec.size() > num_supported_attachments) {
                LOG(ERROR) << "You're trying to use " << texture_outputs_vec.size() << " color attachments with pass "
                           << pass.name << ", but your GPU only supports " << num_supported_attachments;

            } else {
                if(texture_outputs_vec[0].name.find("Backbuffer") != std::string::npos) {
                    last_texture_size = swapchain->get_swapchain_extent();
                    last_texture_name = "Backbuffer";

                } else {
                    LOG(TRACE) << "Getting initial texture '" << texture_outputs_vec[0].name << "' from the texture store";
                    const auto &first_tex = textures.get_texture(texture_outputs_vec[0].name);
                    last_texture_size = first_tex.get_size();
                    last_texture_name = first_tex.get_name();
                }

                for(const auto &color_attachment_info : texture_outputs_vec) {
                    LOG(DEBUG) << "Adding color texture '" << color_attachment_info.name << "'";

                    auto attachment = vk::AttachmentDescription()
                            .setFormat(swapchain->get_swapchain_format())
                            .setSamples(vk::SampleCountFlagBits::e1)
                            .setInitialLayout(vk::ImageLayout::eUndefined)
                            .setStoreOp(vk::AttachmentStoreOp::eStore);

                    if(color_attachment_info.clear) {
                        attachment.setLoadOp(vk::AttachmentLoadOp::eClear);

                    } else {
                        attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
                    }

                    if(color_attachment_info.name.find("Backbuffer") != std::string::npos) {
                        LOG(TRACE) << "Special snowflake backbuffer code path";

                        // The backbuffer is a special snowflake
                        if(swapchain->get_swapchain_extent() != last_texture_size) {
                            LOG(ERROR) << "Backbuffer does not have the same size as texture "
                                       << last_texture_name
                                       << ". In order to use two textures in the same pass, they must have the same size";
                        }

                        last_texture_size = swapchain->get_swapchain_extent();
                        last_texture_name = "Backbuffer";

                        attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

                    } else {
                        LOG(TRACE) << "Getting color attachment info '" << color_attachment_info.name
                                   << "' from the texture store";
                        const auto &texture = textures.get_texture(color_attachment_info.name);

                        if(texture.get_size() != last_texture_size) {
                            LOG(ERROR) << "Texture " << texture.get_name() << " does not have the same size as texture "
                                       << last_texture_name
                                       << ". In order to use two textures in the same pass, they must have the same size";
                        }

                        last_texture_size = texture.get_size();
                        last_texture_name = texture.get_name();

                        attachment.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);
                    }

                    auto color_ref = vk::AttachmentReference()
                            .setAttachment(static_cast<uint32_t>(attachments.size()))
                            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

                    color_refs.push_back(color_ref);
                    attachments.push_back(attachment);
                }
            }
        }

        if(pass.depth_texture) {
            const auto& depth_tex_info = pass.depth_texture.value();
            LOG(DEBUG) << "Adding depth texture " << depth_tex_info.name;
            const auto& depth_tex = textures.get_texture(depth_tex_info.name);

            auto depth_attachment = vk::AttachmentDescription()
                .setFormat(depth_tex.get_format())
                .setSamples(vk::SampleCountFlagBits::e1)
                .setInitialLayout(vk::ImageLayout::eUndefined)
                .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
                .setStencilStoreOp(vk::AttachmentStoreOp::eStore);

            if(depth_tex_info.clear) {
                depth_attachment.setLoadOp(vk::AttachmentLoadOp::eClear);

            } else {
                depth_attachment.setLoadOp(vk::AttachmentLoadOp::eDontCare);
            }

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

        auto renderpass =  context->device.createRenderPass(render_pass_create_info);
        if(renderpass == vk::RenderPass()) {
            LOG(ERROR) << "Failed to create renderpass for pass " << pass.name;
        }

        return renderpass;
    }


    std::tuple<vk::Framebuffer, vk::Extent2D, int32_t> make_framebuffer(const render_pass &pass, const vk::RenderPass renderpass, texture_manager& textures,
                                     std::shared_ptr<render_context> context) {
        LOG(INFO) << "Making framebuffer for pass " << pass.name;
        std::vector<vk::ImageView> attachments;
        vk::Extent2D framebuffer_size;

        bool writes_to_backbuffer = false;
        int32_t depth_idx = -1;

        if(pass.texture_outputs) {
            const auto& outputs = pass.texture_outputs.value();

            for(const auto& output_info : outputs) {
                if(output_info.name != "Backbuffer") {
                    const auto &tex = textures.get_texture(output_info.name);
                    attachments.push_back(tex.get_image_view());
                    framebuffer_size = tex.get_size();

                } else {
                    if(outputs.size() > 1) {
                        LOG(ERROR) << "You're writing to " << outputs.size()
                                   << " attachments, one of which is the backbuffer. If you're writing to the backbuffer, Nova doesn't allow you to write to any other textures";
                        return {};
                    }

                    writes_to_backbuffer = true;
                }
            }
        }
        if(pass.depth_texture) {
            if (writes_to_backbuffer) {
                LOG(ERROR)
                        << "Passes that write to the backbuffer are not allowed to write to a depth buffer. Ignoring depth buffer for pass "
                        << pass.name;
            } else {
                const auto &depth_tex_info = pass.depth_texture.value();
                const auto &depth_tex = textures.get_texture(depth_tex_info.name);

                depth_idx = static_cast<int32_t>(attachments.size());

                attachments.push_back(depth_tex.get_image_view());
                framebuffer_size = depth_tex.get_size();
            }
        }

        if(!attachments.empty()) {
            auto framebuffer_create_info = vk::FramebufferCreateInfo()
                    .setAttachmentCount(static_cast<uint32_t>(attachments.size()))
                    .setPAttachments(attachments.data())
                    .setRenderPass(renderpass)
                    .setWidth(framebuffer_size.width)
                    .setHeight(framebuffer_size.height)
                    .setLayers(1);

            return {context->device.createFramebuffer(framebuffer_create_info), framebuffer_size, depth_idx};
        }

        if(!writes_to_backbuffer) {
            LOG(ERROR) << "No framebuffer attachments for pass " << pass.name << ". This is an error and you should fix it";
        }

        return {vk::Framebuffer(), framebuffer_size, depth_idx};
    }
}
