#pragma once

//! \brief Functions for converting from renderpack data to RHI create info structs
//! Currently there's a very direct correlation between these. If we ship with this strong correlation, I'll feel dirty

#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    RenderPassCreateInfo to_rhi_create_info(const shaderpack::RenderPassCreateInfo& info);

    TextureAttachmentInfo to_rhi_attachment_info(const shaderpack::TextureAttachmentInfo& info);

    PixelFormat to_rhi_pixel_format(const shaderpack::PixelFormatEnum pixel_format);
}
