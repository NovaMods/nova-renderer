#pragma once

#include <rx/core/optional.h>

#include "rhi/pipeline_create_info.hpp"

namespace nova {
    namespace renderer {
        class Rendergraph;

        namespace renderpack {
            struct PipelineData;
        }
    } // namespace renderer
} // namespace nova

namespace nova::renderer::renderpack {

    rx::optional<RhiPipelineState> to_pipeline_state_create_info(const renderpack::PipelineData& data,
                                                                        const Rendergraph& rendergraph);
};
