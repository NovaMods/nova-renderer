#include <easylogging++.h>
#include "geometry_filter.h"

namespace nova {
    and_geometry_filter::and_geometry_filter(std::shared_ptr<igeometry_filter> filter1, std::shared_ptr<igeometry_filter> filter2) : filter1(filter1), filter2(filter2) {}

    bool and_geometry_filter::matches(const mc_block &block) const {
        return filter1->matches(block) && filter2->matches(block);
    }

    bool and_geometry_filter::matches(const render_object &obj) const {
        return filter1->matches(obj) && filter2->matches(obj);
    }

    std::string and_geometry_filter::to_string() const {
        return filter1->to_string() + " AND " + filter2->to_string();
    }

    or_geometry_filter::or_geometry_filter(std::shared_ptr<igeometry_filter> filter1, std::shared_ptr<igeometry_filter> filter2) : filter1(filter1), filter2(filter2) {}

    bool or_geometry_filter::matches(const mc_block &block) const {
        return filter1->matches(block) || filter2->matches(block);
    }

    bool or_geometry_filter::matches(const render_object &obj) const {
        return filter1->matches(obj) || filter2->matches(obj);
    }

    std::string or_geometry_filter::to_string() const {
        return filter1->to_string() + " OR " + filter2->to_string();
    }

    name_geometry_filter::name_geometry_filter(std::string name) : name(name) {}

    bool name_geometry_filter::matches(const mc_block &block) const {
        return std::string(block.name) == name;
    }

    bool name_geometry_filter::matches(const render_object &obj) const {
        return obj.name == name;
    }

    std::string name_geometry_filter::to_string() const {
        return "name::" + name;
    }

    name_part_geometry_filter::name_part_geometry_filter(std::string name_part) : name_part(name_part) {}

    bool name_part_geometry_filter::matches(const mc_block &block) const {
        return std::string(block.name).find(name_part) != std::string::npos;
    }

    bool name_part_geometry_filter::matches(const render_object &obj) const {
        return obj.name.find(name_part) != std::string::npos;
    }

    std::string name_part_geometry_filter::to_string() const {
        return "name_part::" + name_part;
    }

    geometry_type_geometry_filter::geometry_type_geometry_filter(geometry_type type) : type(type) {}

    bool geometry_type_geometry_filter::matches(const mc_block &block) const {
        //LOG(INFO) << "Checking if block is of type " << type.to_string();
        return type == geometry_type::block;
    }

    bool geometry_type_geometry_filter::matches(const render_object &obj) const {
        // We can't make this comparison so let's just ignore it
        return true;
    }

    std::string geometry_type_geometry_filter::to_string() const {
        return "geometry_type::" + type.to_string();
    }

    transparent_geometry_filter::transparent_geometry_filter() : transparent_geometry_filter(true) {}

    transparent_geometry_filter::transparent_geometry_filter(bool should_be_transparent) : should_be_transparent(should_be_transparent) {
        //LOG(INFO) << "Should this filter accept transparent geometry? " << this->should_be_transparent;
    }

    bool transparent_geometry_filter::matches(const mc_block &block) const {
        //LOG(INFO) << "Is the block transparent? " << (block.is_opaque ? "false" : "true") << ". Do we accept transparency? " << (should_be_transparent ? "true" : "false");
        return block.is_transparent() == should_be_transparent;
    }

    bool transparent_geometry_filter::matches(const render_object &obj) const {
        return obj.is_transparent;
    }

    std::string transparent_geometry_filter::to_string() const {
        if(should_be_transparent) {
            return "transparent";
        } else {
            return "not_transparent";
        }
    }

    emissive_geometry_filter::emissive_geometry_filter(bool should_be_emissive) : should_be_emissive(should_be_emissive) {}

    bool emissive_geometry_filter::matches(const mc_block &block) const {
        return block.is_emissive() == should_be_emissive;
    }

    bool emissive_geometry_filter::matches(const render_object &obj) const {
        return obj.is_emissive;
    }

    std::string emissive_geometry_filter::to_string() const {
        if(should_be_emissive) {
            return "emissive";
        } else {
            return "not_emissive";
        }
    }
}

