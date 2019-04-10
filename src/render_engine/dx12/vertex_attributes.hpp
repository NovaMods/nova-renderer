#pragma once

#include <cstdint>
#include <unordered_map>
#include <dxgi.h>

#include "../../util/logger.hpp"

namespace nova::renderer {
    struct vertex_attribute {
        DXGI_FORMAT format;
        uint32_t offset;
    };

    // vec3 Position; vec2 UV0; vec2 MidTexCoord; int VirtualTextureId; vec4 Color; vec2 UV1; vec3 Normal; vec3 Tangent; vec4 McEntityId;
    inline std::unordered_map<vertex_field_enum, vertex_attribute> get_all_vertex_attributes() {
        return {{vertex_field_enum::Position, {DXGI_FORMAT_R32G32B32_FLOAT, 0}},
                {vertex_field_enum::UV0, {DXGI_FORMAT_R32G32_FLOAT, 12}},
                {vertex_field_enum::MidTexCoord, {DXGI_FORMAT_R32G32_FLOAT, 20}},
                {vertex_field_enum::VirtualTextureId, {DXGI_FORMAT_R32_UINT, 28}},
                {vertex_field_enum::Color, {DXGI_FORMAT_R32G32B32A32_FLOAT, 32}},
                {vertex_field_enum::UV1, {DXGI_FORMAT_R32G32_FLOAT, 48}},
                {vertex_field_enum::Normal, {DXGI_FORMAT_R32G32B32_FLOAT, 56}},
                {vertex_field_enum::Tangent, {DXGI_FORMAT_R32G32B32_FLOAT, 68}},
                {vertex_field_enum::McEntityId, {DXGI_FORMAT_R32G32B32A32_FLOAT, 80}}};
    }

    inline uint32_t get_total_vertex_size() {
        uint32_t total_size = 0;
        for(const auto& attribute : get_all_vertex_attributes()) {
            uint32_t size;
            switch(attribute.second.format) {
                case DXGI_FORMAT_R32_UINT:
                    size = 4;
                    break;
                case DXGI_FORMAT_R32_FLOAT:
                    size = 4;
                    break;
                case DXGI_FORMAT_R32G32_FLOAT:
                    size = 8;
                    break;
                case DXGI_FORMAT_R32G32B32_FLOAT:
                    size = 12;
                    break;
                case DXGI_FORMAT_R32G32B32A32_FLOAT:
                    size = 16;
                    break;
                default:
                    NOVA_LOG(ERROR) << "Unknown format " << attribute.second.format;
                    throw std::runtime_error("Unknown format");
            }
            total_size += size;
        }

        return total_size;
    }
} // namespace nova::renderer
