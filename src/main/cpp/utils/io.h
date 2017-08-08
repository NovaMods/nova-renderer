/*! 
 * \author gold1 
 * \date 09-Jun-17.
 */

#ifndef RENDERER_IO_H
#define RENDERER_IO_H

#include <json.hpp>
#include "../mc_interface/mc_objects.h"

namespace nova {
    nlohmann::json to_json(const mc_block& block);

    void from_json(nlohmann::json& j, mc_block& block);


    nlohmann::json to_json(const mc_chunk_render_object& chunk);

    void from_json(nlohmann::json& j, mc_chunk_render_object& chunk);


    void save_chunk(const mc_chunk_render_object& chunk, const std::string filename);

    std::shared_ptr<mc_chunk_render_object> load_chunk(const std::string filename);
}


#endif //RENDERER_IO_H
