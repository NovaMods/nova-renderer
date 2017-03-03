#include "geometry_filter.h"

namespace nova {
    // CLion says these lines are an error. CLion is stupid
    std::unordered_map<std::string, std::function<void(geometry_filter&)>> geometry_filter::modifying_functions {
            { "solid", accept_solid },
            { "not_solid", reject_solid },
            { "transparent", accept_transparent },
            { "not_transparent", reject_transparent },
            { "cutout", accept_cutout },
            { "not_cutout", reject_cutout },
            { "emissive", accept_emissive },
            { "not_emissive", reject_emissive },
            { "damaged", accept_damaged },
            { "not_damaged", reject_damaged },
            { "everything_else", accept_everything_else },
            { "nothing_else", reject_everything_else }
    };

    void accept_geometry_type(geometry_filter &filter, geometry_type type) {
        filter.geometry_types.push_back(type);
    }

    void accept_name(geometry_filter &filter, std::string &name) {
        filter.names.push_back(name);
    }

    void accept_name_part(geometry_filter &filter, std::string &name_part) {
        filter.name_parts.push_back(name_part);
    }

    void accept_solid(geometry_filter &filter) {
        filter.should_be_solid = true;
    }

    void reject_solid(geometry_filter &filter) {
        filter.should_be_solid = false;
    }

    void accept_transparent(geometry_filter &filter) {
        filter.should_be_transparent = true;
    }

    void reject_transparent(geometry_filter &filter) {
        filter.should_be_transparent = false;
    }

    void accept_cutout(geometry_filter &filter) {
        filter.should_be_cutout = true;
    }

    void reject_cutout(geometry_filter &filter) {
        filter.should_be_cutout = false;
    }

    void accept_emissive(geometry_filter &filter) {
        filter.should_be_emissive = true;
    }

    void reject_emissive(geometry_filter &filter) {
        filter.should_be_emissive = false;
    }

    void accept_damaged(geometry_filter &filter) {
        filter.should_be_damaged = true;
    }

    void reject_damaged(geometry_filter &filter) {
        filter.should_be_damaged = false;
    }

    void accept_everything_else(geometry_filter &filter) {
        if(!filter.should_be_solid) filter.should_be_solid = false;
        if(!filter.should_be_transparent) filter.should_be_transparent = false;
        if(!filter.should_be_cutout) filter.should_be_cutout = false;
        if(!filter.should_be_emissive) filter.should_be_emissive = false;
        if(!filter.should_be_damaged) filter.should_be_damaged = false;
    }

    void reject_everything_else(geometry_filter &filter) {
        if(!filter.should_be_solid) filter.should_be_solid = true;
        if(!filter.should_be_transparent) filter.should_be_transparent = true;
        if(!filter.should_be_cutout) filter.should_be_cutout = true;
        if(!filter.should_be_emissive) filter.should_be_emissive = true;
        if(!filter.should_be_damaged) filter.should_be_damaged = true;
    }

    bool matches_filter(render_object& object, geometry_filter &filter) {
        for(auto& name : filter.names) {
            if(object.name == name) {
                return true;
            }
        }

        for(auto& name_part : filter.name_parts) {
            if(object.name.find(name_part) != std::string::npos) {
                return true;
            }
        }

        bool matches = false;
        bool matches_geometry_type = false;
        for(auto& geom_type : filter.geometry_types) {
            if(object.type == geom_type) {
                matches_geometry_type = true;
            }
        }

        matches |= matches_geometry_type;
        if(filter.geometry_types.size() == 0) {
            matches = true;
        }

        if(filter.should_be_solid) {
            matches |= *filter.should_be_solid && object.is_solid;
        }
        if(filter.should_be_transparent) {
            matches |= *filter.should_be_transparent && object.is_transparent;
        }
        if(filter.should_be_cutout) {
            matches |= *filter.should_be_cutout && object.is_cutout;
        }
        if(filter.should_be_emissive) {
            matches |= *filter.should_be_emissive&& object.is_emissive;
        }
        if(filter.should_be_damaged) {
            matches |= *filter.should_be_damaged ? object.damage_level > 0 : object.damage_level == 0;
        }

        return matches;
    }
}

