#include "geometry_fitler.h"

namespace nova {
    namespace view {
        void accept_block(geometry_filter& filter) {
            filter.accept_block = true;
        }

        void reject_block(geometry_filter& filter) {
            filter.accept_block = false;
        }

        void accept_entity(geometry_filter& filter) {
            filter.accept_entity = true;
        }

        void reject_entity(geometry_filter& filter) {
            filter.accept_entity = false;
        }

        void accept_selection_box(geometry_filter& filter) {
            filter.accept_selection_box = true;
        }

        void reject_selection_box(geometry_filter& filter) {
            filter.accept_selection_box = false;
        }

        void accept_particle(geometry_filter& filter) {
            filter.accept_particle = true;
        }

        void reject_particle(geometry_filter& filter) {
            filter.accept_particle = false;
        }

        void accept_sky_object(geometry_filter& filter) {
            filter.accept_sky_object = true;
        }

        void reject_sky_object(geometry_filter& filter) {
            filter.accept_sky_object = false;
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
    }
}

