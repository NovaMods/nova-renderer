#ifndef GEOMETRY_FILTER_H
#define GEOMETRY_FILTER_H

namespace nova {
    namespace view {
        geometry_filter& geometry_filter::block() {
            should_be_block = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_block() {
            should_be_block = false;
            return *this;
        }

        geometry_filter &geometry_filter::entity() {
            should_be_entity = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_entity() {
            should_be_entity = false;
            return *this;
        }

        geometry_filter &geometry_filter::particle() {
            should_be_particle = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_particle() {
            should_be_particle = false;
            return *this;
        }

        geometry_filter &geometry_filter::sky_object() {
            should_be_sky_object = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_sky_object() {
            should_be_sky_object = false;
            return *this;
        }

        geometry_filter &geometry_filter::add_geometry_type(geometry_type type) {
            geometry_types.push_back(type);
            return *this;
        }

        geometry_filter &geometry_filter::add_name(std::string name) {
            names.push_back(name);
            return *this;
        }

        geometry_filter &geometry_filter::add_name_part(std::string name_part) {
            name_parts.puch_back(name_part);
            return *this;
        }

        geometry_filter &geometry_filter::transparent() {
            should_be_transparent = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_transparent() {
            should_be_transparent = false;
            return *this;
        }

        geometry_filter &geometry_filter::cutout() {
            should_be_cutout = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_cutout() {
            should_be_cutout = false;
            return *this;
        }

        geometry_filter &geometry_filter::emissive() {
            should_be_emissive = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_emissive() {
            should_be_emissive = false;
            return *this;
        }

        geometry_filter geometry_filter::build() {
            return *this;
        }

        geometry_filter &geometry_filter::damaged() {
            should_be_damaged = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_damaged() {
            should_be_damaged = false;
            return *this;
        }

        geometry_filter &geometry_filter::selection_box() {
            should_be_selection_box = true;
            return *this;
        }

        geometry_filter &geometry_filter::not_selection_box() {
            should_be_selection_box = false;
            return *this;
        }
    };
};

#endif
