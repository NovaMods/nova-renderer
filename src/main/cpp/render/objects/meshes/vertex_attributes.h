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

    std::unordered_map<std::string, vertex_attribute> all_vertex_attributes = {
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

    inline uint32_t get_total_vertex_size() {
        uint32_t tital_size = 0;
        for(const auto& attribute : all_vertex_attributes) {
            auto size;
            switch(attribute.format) {
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
                    LOG(ERROR) << "Unknown format " << vk::to_string(attribute.format);
                    throw std::runtime_error("Unknown format");
            }
        }
    }
}

#endif //NOVA_RENDERER_VERTEX_ATTRIBUTES_H
