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

    nlohmann::json to_json(const mc_basic_render_object& chunk) {
        auto j = nlohmann::json{
                {"chunk_id", chunk.chunk_id}
        };

        nlohmann::json blocks;
        int count = 0;
        for(int i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_DEPTH; i++) {
            nlohmann::json block_json = to_json(chunk.blocks[i]);
            blocks.push_back(block_json);
            count++;
        }

        j["blocks"] = blocks;

        return j;
    }

    void from_json(nlohmann::json& j, mc_basic_render_object& chunk) {
        chunk.chunk_id = j["chunk_id"];

        for(int i = 0; i < j["blocks"].size(); i++) {
            mc_block block;
            from_json(j["blocks"][i], block);
            chunk.blocks[i] = block;
        }
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
