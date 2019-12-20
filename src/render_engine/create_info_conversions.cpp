#include "create_info_conversions.hpp"

namespace nova::renderer::rhi {
    RenderPassCreateInfo to_rhi_create_info(const shaderpack::RenderPassCreateInfo& info) {
        RenderPassCreateInfo rhi_info = {};
        rhi_info.name = info.name;
        rhi_info.dependencies = info.dependencies;
        rhi_info.texture_inputs = info.texture_inputs;
        rhi_info.texture_outputs.reserve(info.texture_outputs.size());
        std::transform(info.texture_outputs.begin(),
                       info.texture_outputs.end(),
                       std::back_inserter(rhi_info.texture_outputs),
                       to_rhi_attachment_info);

        rhi_info.dependencies = info.dependencies;
        rhi_info.input_buffers = info.input_buffers;
        rhi_info.output_buffers = info.output_buffers;

        return rhi_info;
    }

    TextureAttachmentInfo to_rhi_attachment_info(const shaderpack::TextureAttachmentInfo& info) {
        TextureAttachmentInfo rhi_info = {};
        rhi_info.name = info.name;
        rhi_info.pixel_format = to_rhi_pixel_format(info.pixel_format);
        rhi_info.clear = info.clear;

        return rhi_info;
    }

    PixelFormat to_rhi_pixel_format(const shaderpack::PixelFormatEnum pixel_format) {
        switch(pixel_format) {
            case shaderpack::PixelFormatEnum::RGBA8:
                return PixelFormat::RGBA8;

            case shaderpack::PixelFormatEnum::RGBA16F:
                return PixelFormat::RGBA16F;

            case shaderpack::PixelFormatEnum::RGBA32F:
                return PixelFormat::RGBA32F;

            case shaderpack::PixelFormatEnum::Depth:
                return PixelFormat::Depth;

            case shaderpack::PixelFormatEnum::DepthStencil:
                return PixelFormat::DepthStencil;

            default:
                return PixelFormat::RGBA8;
        }
    }
} // namespace nova::renderer::rhi
