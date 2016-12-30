#include "geometry_fitler.h"

namespace nova {
    namespace view {
        void accept_block(geometry_filter& filter) {
            filter.should_be_block = true;
        }

        void reject_block(geometry_filter& filter) {
            filter.should_be_block = false;
        }

        void accept_entity(geometry_filter& filter) {
            filter.should_be_entity = true;
        }

        void reject_entity(geometry_filter& filter) {
            filter.should_be_entity = false;
        }

        void accept_selection_box(geometry_filter& filter) {
            filter.should_be_selection_box = true;
        }

        void reject_selection_box(geometry_filter& filter) {
            filter.should_be_selection_box = false;
        }

        void accept_particle(geometry_filter& filter) {
            filter.should_be_particle = true;
        }

        void reject_particle(geometry_filter& filter) {
            filter.should_be_particle = false;
        }

        void accept_sky_object(geometry_filter& filter) {
            filter.should_be_sky_object = true;
        }

        void reject_sky_object(geometry_filter& filter) {
            filter.should_be_sky_object = false;
        }

        void accept_geometry_type(geometry_filter& filter, geometry_type type) {
            filter.geometry_types.push_back(type);
        }

        void accept_name(geometry_filter& fitler, std::string& name) {
            filter.names.push_back(name);
        }

        void accept_name_part(geometry_filter& filter, std::string& name_part) {
            filter.name_parts.push_back(name_part);
        }

        void accept_solid(geometry_filter& filter) {
            filter.should_be_solid = true;
        }

        void reject_solid(geometry_filter& filter) {
            filter.should_be_solid = false;
        }

        void accept_transparent(geometry_filter& filter) {
            filter.should_be_transparent = true;
        }

        void reject_transparent(geometry_filter& filter) {
            filter.should_be_transparent = false;
        }

        void accept_cutout(geometry_filter& filter) {
            filter.should_be_cutout = true;
        }

        void reject_cutout(geometry_filter& filter) {
            filter.should_be_cutout = false;
        }

        void accept_emissive(geometry_filter& filter) {
            filter.should_be_emissive = true;
        }

        void reject_emissive(geometry_filter& filter) {
            filter.should_be_emissive = false;
        }

        void accept_damaged(geometry_filter& filter) {
            filter.should_be_damaged = true;
        }

        void reject_damaged(geometry_filter& filter) {
            filter.should_be_damaged = false;
        }

        void accept_everything_else(geometry_filter& filter) {
            if(!filter.should_be_block) filter.should_be_block = false;
            if(!filter.should_be_selection_box) filter.should_be_selection_box = false;
            if(!filter.should_be_entity) filter.should_be_entity = false;
            if(!filter.should_be_particle) filter.should_be_particle = false;
            if(!filter.should_be_sky_object) filter.should_be_sky_object = false;

            if(!filter.should_be_solid) filter.should_be_solid = false;
            if(!filter.should_be_transparent) filter.should_be_transparent = false;
            if(!filter.should_be_cutout) filter.should_be_cutout = false;
            if(!filter.should_be_emissive) filter.should_be_emissive = false;
            if(!filter.should_be_damaged) filter.should_be_damaged = false;
        }

        void reject_everything_else(geometry_filter& filter) {
            if(!filter.should_be_block) filter.should_be_block = true;
            if(!filter.should_be_selection_box) filter.should_be_selection_box = true;
            if(!filter.should_be_entity) filter.should_be_entity = true;
            if(!filter.should_be_particle) filter.should_be_particle = true;
            if(!filter.should_be_sky_object) filter.should_be_sky_object = true;

            if(!filter.should_be_solid) filter.should_be_solid = true;
            if(!filter.should_be_transparent) filter.should_be_transparent = true;
            if(!filter.should_be_cutout) filter.should_be_cutout = true;
            if(!filter.should_be_emissive) filter.should_be_emissive = true;
            if(!filter.should_be_damaged) filter.should_be_damaged = true;
        }
    }
}

