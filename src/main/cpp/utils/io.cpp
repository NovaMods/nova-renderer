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
                {"name", block.name},
                {"is_on_fire", block.is_on_fire},
                {"light_value", block.light_value},
                {"light_opacity", block.light_opacity},
                {"ao", block.ao},
                {"is_opaque", block.is_opaque},
                {"blocks_light", block.blocks_light}
        };
    }

    void from_json(nlohmann::json& j, mc_block& block) {
        std::string *name = new std::string{j["name"].get<std::string>()};
        block.name = name->data();
        block.is_on_fire = j["is_on_fire"].get<bool>();
        block.light_value = j["light_value"].get<int>();
        block.light_opacity = j["light_opacity"].get<int>();
        block.ao = j["ao"].get<float>();
        block.is_opaque = j["is_opaque"].get<bool>();
        block.blocks_light = j["blocks_light"].get<bool>();
    }

    nlohmann::json to_json(const mc_chunk& chunk) {
        auto j = nlohmann::json{
                {"chunk_id", chunk.chunk_id}
        };

        nlohmann::json blocks;
        int count = 0;
        for(const auto& block : chunk.blocks) {
            nlohmann::json block_json = to_json(block);
            blocks.push_back(block_json);
            count++;
        }

        j["blocks"] = blocks;

        return j;
    }

    void from_json(nlohmann::json& j, mc_chunk& chunk) {
        chunk.chunk_id = j["chunk_id"];

        for(int i = 0; i < j["blocks"].size(); i++) {
            mc_block block;
            from_json(j["blocks"][i], block);
            chunk.blocks[i] = block;
        }
    }

    void save_chunk(const mc_chunk& chunk, const std::string filename) {
        std::ofstream out(filename);
        nlohmann::json j = to_json(chunk);
        out << j.dump();
        LOG(INFO) << "Wrote JSON to file";
    }

    std::shared_ptr<mc_chunk> load_chunk(const std::string filename) {
        std::ifstream file(filename);
        nlohmann::json j;
        file >> j;

        mc_chunk* chunk = new mc_chunk();
        from_json(j, *chunk);
        return std::make_shared<mc_chunk>(*chunk);
    }
}
