/*!
 * \brief Holds code to generate a texture atlas
 */

#ifndef ATLAS_GENERATING_H
#define ATLAS_GENERATING_H

#include <memory>
#include "../loaders/texture_loading.h"

namespace nova {
    namespace model {
        struct rectangle {
            int x;
            int y;
            int width;
            int height;
        };

        class atlas_node {
        public:
            atlas_node(const rectangle& rect);

            bool is_leaf() const noexcept;

            std::unique_ptr<atlas_node> insert(texture_data texture, int padding);

        private:
            rectangle rect;
            texture_data data;
            std::unique_ptr<atlas_node> left;
            std::unique_ptr<atlas_node> right;
        };

        class atlas_texture {
        private:
            atlas_node node;
            std::unordered_map<std::string, rectangle> uv_coords;
        };
    }
}

#endif
