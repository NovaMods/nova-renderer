/*! 
 * \author gold1 
 * \date 09-Jun-17.
 */

#include <fstream>
#include "io.h"
#include <easylogging++.h>

namespace nova {
    nlohmann::json to_json(const mc_block& block) {
        return nlohmann::json{
                {"id", block.id},
                {"is_on_fire", block.is_on_fire},
                {"ao", block.ao},
        };
    }

    void from_json(nlohmann::json& j, mc_block& block) {
        block.is_on_fire = j["is_on_fire"].get<bool>();
        block.ao = j["ao"].get<float>();
        block.id = j["id"].get<int>();
    }

    void save_chunk(const mc_basic_render_object& chunk, const std::string filename) {
        std::ofstream out(filename);
        nlohmann::json j = to_json(chunk);
        out << j.dump();
    }

    std::shared_ptr<mc_basic_render_object> load_chunk(const std::string filename) {
        std::ifstream file(filename);
        nlohmann::json j;
        file >> j;

        mc_basic_render_object* chunk = new mc_basic_render_object();
        from_json(j, *chunk);
        return std::make_shared<mc_basic_render_object>(*chunk);
    }
}
