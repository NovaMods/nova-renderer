//
// Created by jannis on 11.09.18.
//

#include "render_object.hpp"

namespace nova {
    render_object::render_object(const std::shared_ptr<render_object> &parent) : parent(parent) {
        parent->children.push_back(shared_from_this());
    }

    render_object::~render_object() {
        if(auto parent = get_parent()) {
            parent->children.remove(shared_from_this());
        }

        for(auto &child : children) {
            child.reset();
        }
    }

    std::shared_ptr<render_object> render_object::get_parent() const {
        return parent.lock();
    }

    const std::list<std::shared_ptr<render_object>> &render_object::get_children() const {
        return children;
    }

    render_object &render_object::pos(const glm::vec3 &pos) {
        _pos = pos;
        return *this;
    }

}  // namespace nova
