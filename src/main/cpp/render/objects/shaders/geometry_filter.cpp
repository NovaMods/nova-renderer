#include "geometry_filter.h"

namespace nova {
    // CLion says these lines are an error. CLion is stupid
    std::unordered_map<std::string, std::function<void(geometry_filter&)>> geometry_filter::modifying_functions {
            { "transparent", accept_transparent },
            { "not_transparent", reject_transparent },
            { "emissive", accept_emissive },
            { "not_emissive", reject_emissive },
            { "everything_else", accept_everything_else },
            { "nothing_else", reject_everything_else }
    };

    bool geometry_filter::matches(const render_object &object) const {
        for(auto& name : names) {
            if(object.name == name) {
                return true;
            }
        }

        for(auto& name_part : name_parts) {
            if(object.name.find(name_part) != std::string::npos) {
                return true;
            }
        }

        bool matches = false;
        bool matches_geometry_type = false;
        for(auto& geom_type : geometry_types) {
            if(object.type == geom_type) {
                matches_geometry_type = true;
            }
        }

        matches |= matches_geometry_type;
        if(geometry_types.size() == 0) {
            matches = true;
        }

        if(should_be_transparent) {
            matches |= *should_be_transparent && object.is_transparent;
        }
        if(should_be_emissive) {
            matches |= *should_be_emissive&& object.is_emissive;
        }

        return matches;
    }

    bool geometry_filter::matches(const mc_block &block) const {
        if(std::find_if(names.begin(), names.end(), [&](auto& name) {return name == block.name;}) != names.end()) {
            return true;
        }

        for(auto& name_part : name_parts) {
            if(std::string(block.name).find(name_part) != std::string::npos) {
                return true;
            }
        }

        bool matches = false;
        bool matches_geometry_type = false;
        for(auto& geom_type : geometry_types) {
            if(geometry_type::block == geom_type) {
                matches_geometry_type = true;
            }
        }

        matches |= matches_geometry_type;
        if(geometry_types.size() == 0) {
            matches = true;
        }

        if(should_be_transparent) {
            matches |= *should_be_transparent && block.is_transparent();
        }
        if(should_be_emissive) {
            matches |= *should_be_emissive && block.is_emissive();
        }

        return matches;
    }

    void accept_geometry_type(geometry_filter &filter, geometry_type type) {
        filter.geometry_types.push_back(type);
    }

    void accept_name(geometry_filter &filter, std::string &name) {
        filter.names.push_back(name);
    }

    void accept_name_part(geometry_filter &filter, std::string &name_part) {
        filter.name_parts.push_back(name_part);
    }

    void accept_transparent(geometry_filter &filter) {
        filter.should_be_transparent = true;
    }

    void reject_transparent(geometry_filter &filter) {
        filter.should_be_transparent = false;
    }


    void accept_everything_else(geometry_filter &filter) {
        if(!filter.should_be_transparent) filter.should_be_transparent = false;
        if(!filter.should_be_emissive) filter.should_be_emissive = false;
    }

    void reject_everything_else(geometry_filter &filter) {
        if(!filter.should_be_transparent) filter.should_be_transparent = true;
        if(!filter.should_be_emissive) filter.should_be_emissive = true;
    }
}

