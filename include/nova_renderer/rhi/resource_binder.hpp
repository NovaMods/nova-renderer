#pragma once

#include <string>
#include <vector>

#include "nova_renderer/rhi/forward_decls.hpp"

namespace nova::renderer {
    /*!
     * \brief Abstraction for making resources available to shaders
     *
     * Resource binders are meant to be long-lived. You should create them for your materials as early as possible and simply use them over
     * and over and over
     */
    class RhiResourceBinder {
    public:
        virtual ~RhiResourceBinder() = default;

        virtual void bind_image(const std::string& binding_name, rhi::RhiImage* image) = 0;

        virtual void bind_buffer(const std::string& binding_name, rhi::RhiBuffer* buffer) = 0;

        virtual void bind_sampler(const std::string& binding_name, rhi::RhiSampler* sampler) = 0;

        virtual void bind_image_array(const std::string& binding_name, const std::vector<rhi::RhiImage*>& images) = 0;

        virtual void bind_buffer_array(const std::string& binding_name, const std::vector<rhi::RhiBuffer*>& buffers) = 0;

        virtual void bind_sampler_array(const std::string& binding_name, const std::vector<rhi::RhiSampler*>& samplers) = 0;
    };
} // namespace nova::renderer
