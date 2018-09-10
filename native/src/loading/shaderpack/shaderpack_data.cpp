/*!
 * \author ddubois 
 * \date 23-Aug-18.
 */

#include "shaderpack_data.hpp"
#include "../utils.hpp"

namespace nova {
    /*!
    * \brief If a data member isn't in the JSON (which is fully supported and is 100% fine) then we use this to fill in
    * any missing values
    */
    pipeline_data default_pipeline;

    pixel_format_enum pixel_format_enum_from_string(const std::string& str) {
        if(str == "RGB8") {
            return pixel_format_enum::RGB8;

        } else if(str == "RGBA8") {
            return pixel_format_enum::RGBA8;

        } else if(str == "RGB16F") {
            return pixel_format_enum::RGB16F;

        } else if(str == "RGBA16F") {
            return pixel_format_enum::RGBA16F;

        } else if(str == "RGB32F") {
            return pixel_format_enum::RGB32F;

        } else if(str == "RGBA32F") {
            return pixel_format_enum::RGBA32F;

        } else if(str == "Depth") {
            return pixel_format_enum::Depth;

        } else if(str == "DepthStencil") {
            return pixel_format_enum::DepthStencil;

        } else {
            NOVA_LOG(ERROR) << "Unsupported pixel format " << str;
            throw parse_failed("Unsupported pixel format " + str);
        }
    }
}
