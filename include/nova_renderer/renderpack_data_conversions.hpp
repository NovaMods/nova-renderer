#pragma once
#include "rhi/pipeline_create_info.hpp"

namespace nova {
    namespace renderer {
        namespace shaderpack {
            struct PipelineData;
        }
    } // namespace renderer
} // namespace nova

namespace nova::renderer::renderpack {

    PipelineStateCreateInfo to_pipeline_state_create_info(const shaderpack::PipelineData& data);
};
