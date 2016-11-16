/*!
 * \brief
 *
 * \author ddubois 
 * \date 15-Nov-16.
 */

#ifndef RENDERER_LOADED_RESOURCE_H
#define RENDERER_LOADED_RESOURCE_H

#include <mutex>
#include <atomic>

namespace nova {
    namespace model {
        template<typename Resource>
        class loaded_resource {
        public:
            template <typename Resource>
            void set_resource(Resource new_resource) {
                lock.lock();
                resource = new_resource;
                has_new = true;
                lock.unlock();
            }

            bool has_new_resource() {
                return has_new;
            }

            template <typename Resource>
            Resource get_resource() {
                Resource copy;
                lock.lock();
                copy = resource;
                has_new = false;
                lock.unlock();

                return copy;
            }

        private:
            std::mutex lock;
            std::atomic<bool> has_new;
            Resource resource;
        };
    }
}

#endif //RENDERER_LOADED_RESOURCE_H
