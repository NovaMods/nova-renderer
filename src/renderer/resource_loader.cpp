#include "nova_renderer/frontend/resource_loader.hpp"

#include "nova_renderer/util/logger.hpp"

namespace nova::renderer {
    ResourceFactory::ResourceFactory(rhi::RenderEngine& device) : device(device) {}

    rhi::Image* ResourceFactory::add_texture(
        const std::string& name, std::size_t width, std::size_t height, rhi::PixelFormat pixel_format, void* data) {
        
    }

    rhi::Image* ResourceFactory::get_texture(const std::string& name) const {
#if NOVA_DEBUG
        if(const auto& itr = textures.find(name); itr != textures.end()) {
            return itr->second;
        } else {
            NOVA_LOG(ERROR) << "Could not find image \"" << name << "\"";
            return nullptr;
        }
#else
        return textures.at(name);
#endif
    }
} // namespace nova::renderer
