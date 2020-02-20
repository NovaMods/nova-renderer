#pragma once

#include <rx/core/optional.h>

#include "rhi/pipeline_create_info.hpp"

namespace nova {
    namespace renderer {
        class Rendergraph;

        namespace shaderpack {
            struct PipelineData;
        }
    } // namespace renderer
} // namespace nova

namespace nova::renderer::renderpack {

    rx::optional<PipelineStateCreateInfo> to_pipeline_state_create_info(const shaderpack::PipelineData& data,
                                                                        const Rendergraph& rendergraph);
};
