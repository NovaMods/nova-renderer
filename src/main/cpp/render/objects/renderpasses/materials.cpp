/*!
 * \author ddubois 
 * \date 06-Nov-17.
 */

#include "materials.h"

namespace nova {
    state_enum decode_state(std::string& state_to_decode);

    material_state create_from_json(const nlohmann::json& material_json, const std::string& name, const std::string& parent) {
        material_state ret_val = {};

        ret_val.name = name;
        ret_val.parent = parent;

        const auto& itr = material_json.find("states");
        if(itr != material_json.end()) {
            auto& states = material_json.at("states");
            for(auto& state : states) {
                state_enum decoded_state = decode_state(state);
            }
        }


        return ret_val;
    }
}
