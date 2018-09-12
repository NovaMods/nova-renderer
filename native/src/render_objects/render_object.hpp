//
// Created by jannis on 11.09.18.
//

#ifndef NOVA_RENDERER_RENDER_OBJECT_HPP
#define NOVA_RENDERER_RENDER_OBJECT_HPP

#include <glm/glm.hpp>
#include <memory>
#include <list>

namespace nova {
    class render_object : public std::enable_shared_from_this<render_object> {
    public:
        explicit render_object() = default;
        explicit render_object(const std::shared_ptr<render_object> &parent);
        ~render_object();

        std::shared_ptr<render_object> get_parent() const;
        const std::list<std::shared_ptr<render_object>> &get_children() const;

        constexpr render_object &pos(const glm::vec3 &pos);

    private:
        std::weak_ptr<render_object> parent;
        std::list<std::shared_ptr<render_object>> children;

        glm::vec3 _pos;
    };
}


#endif //NOVA_RENDERER_RENDER_OBJECT_HPP
