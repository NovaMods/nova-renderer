#include "pipeline_reflection.hpp"

#include "spirv_cross.hpp"

namespace nova::renderer {
    void get_bindings_from_shader(const rx::vector<uint32_t>& spirv, rx::map<rx::string, ResourceBindingLocation>& bindings) {
        const spirv_cross::Compiler compiler{spirv.data(), spirv.size()};

        const auto& images = compiler.get_shader_resources().sampled_images;
        for(const auto& image : images) {
            rx::string name{bindings.allocator(), image.name.data()};

            const auto set = compiler.get_decoration(image.id, spv::DecorationDescriptorSet);
            const auto binding = compiler.get_decoration(image.id, spv::DecorationBinding);

            bindings.insert(name, {set, binding});
        }
    }

    rx::map<rx::string, ResourceBindingLocation> get_bespoke_image_binding_locations(const RhiGraphicsPipelineState& pipeline_state,
                                                                                     rx::memory::allocator& allocator) {
        rx::map<rx::string, ResourceBindingLocation> binding_locations{&allocator};

        get_bindings_from_shader(pipeline_state.vertex_shader.source, binding_locations);

        if(pipeline_state.geometry_shader) {
            get_bindings_from_shader(pipeline_state.geometry_shader->source, binding_locations);
        }

        if(pipeline_state.pixel_shader) {
            get_bindings_from_shader(pipeline_state.pixel_shader->source, binding_locations);
        }

        return binding_locations;
    }
} // namespace nova::renderer
