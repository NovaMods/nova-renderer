/*!
 * \author ddubois 
 * \date 31-Mar-18.
 */

#ifndef NOVA_RENDERER_VERTEX_ATTRIBUTES_H
#define NOVA_RENDERER_VERTEX_ATTRIBUTES_H

#include <vulkan/vulkan.hpp>
#include <string>
#include <unordered_map>

namespace nova {
    struct vertex_attribute {
        vk::Format format;
        uint32_t offset;
    };

    inline std::unordered_map<std::string, vertex_attribute> get_all_vertex_attributes() {
        return {
            { "Position",           {vk::Format::eR32G32B32Sfloat,       0} },
            { "UV0",                {vk::Format::eR32G32Sfloat,         12} },
            { "MidTexCoord",        {vk::Format::eR32G32Sfloat,         20} },
            { "VirtualTextureId",   {vk::Format::eR32Uint,              28} },
            { "Color",              {vk::Format::eR32G32B32A32Sfloat,   32} },
            { "UV1",                {vk::Format::eR32G32Sfloat,         48} },
            { "Normal",             {vk::Format::eR32G32B32Sfloat,      56} },
            { "Tangent",            {vk::Format::eR32G32B32Sfloat,      68} },
            { "McEntityId",         {vk::Format::eR32G32B32A32Sfloat,   80} }
        };
    }

    inline uint32_t get_total_vertex_size() {
        uint32_t total_size = 0;
        for(const auto& attribute : get_all_vertex_attributes()) {
            uint32_t size;
            switch(attribute.second.format) {
                case (vk::Format::eR32Uint):
                    size = 4;
                    break;
                case (vk::Format::eR32Sfloat):
                    size = 8;
                    break;
                case (vk::Format::eR32G32Sfloat):
                    size = 16;
                    break;
                case (vk::Format::eR32G32B32Sfloat):
                    size = 32;
                    break;
                default:
                    LOG(ERROR) << "Unknown format " << vk::to_string(attribute.second.format);
                    throw std::runtime_error("Unknown format");
            }
            total_size += size;
        }

        return total_size;
    }
}

#endif //NOVA_RENDERER_VERTEX_ATTRIBUTES_H
