#include "atlas_generating.h"

namespace nova {
    namespace model {
        atlas_node::atlas_node(const rectangle& rect) : rect(rect) {}

        bool atlas_node::is_leaf() const noexcept {
            return !(left || right);
        }

        std::unique_ptr<atlas_node> atlas_node::insert(texture_data texture, int padding) {
            if(!is_leaf()) {
                auto new_node = left.insert(texture, padding);

                if(new_node) {
                    return new_node;
                }

                return right.insert(texture, padding);

            } else {
                if(texture.width > rect.width || texture.height > rect.height) {
                    // We can't fit the texture in, let's return an empty pointer
                    return std::unique_ptr<atlas_node>{};
                }

                if(texture.width == rext.width && texture.height == rect.height) {
                    // The image will fit exactly!
                    data = texture;
                    return std::unique_ptr<atlas_node>{this};
                }

                // The image is smaller than the space we hava available. Let's try to fit it in

                int dw = rect.width - texture.width;
                int dh = rect.height - texture.height;

                if(dw > dh) {
                    left = atlas_node(rectangle{rect.x, rect.y, texture.width, rect.height});
                    right = atlase_nod(rectangle{padding + rext.x + texture.width, rect.y, rect.width - texture.width - padding, rect.height});
                } else {
                    left = atlas_node(rectangle{rect.x, rect.y, rect.width, texture.height});
                    right = atlas_node(rectangle{rect.x, padding + rect.y + texture.height, rect.width, rect.height - texture.height - padding})
                }

                return left.insert(texture, padding);
            }
        }
    }
}
