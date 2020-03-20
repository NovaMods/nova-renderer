#pragma once

#include <rx/core/string.h>
#include <rx/core/vector.h>

#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    class RhiResourceBinder {
    public:
        virtual ~RhiResourceBinder() = default;

        virtual void bind_image(const rx::string& binding_name, rhi::RhiImage* image) = 0;

        virtual void bind_buffer(const rx::string& binding_name, rhi::RhiBuffer* buffer) = 0;

        virtual void bind_sampler(const rx::string& binding_name, rhi::RhiSampler* sampler) = 0;

        virtual void bind_image_array(const rx::string& binding_name, const rx::vector<rhi::RhiImage*>& images) = 0;

        virtual void bind_buffer_array(const rx::string& binding_name, const rx::vector<rhi::RhiBuffer*>& buffers) = 0;

        virtual void bind_sampler_array(const rx::string& binding_name, const rx::vector<rhi::RhiSampler*>& samplers) = 0;
    };
} // namespace nova::renderer
