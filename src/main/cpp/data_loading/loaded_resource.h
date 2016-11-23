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
            loaded_resource() {}

            loaded_resource(const loaded_resource<Resource> &other) {
                has_new = other.has_new;
                resource = other.resource;
            }

            loaded_resource(loaded_resource<Resource> &other) {
                has_new = other.has_new;
                resource = other.resource;
            }

            void set_resource(Resource new_resource) {
                lock.lock();
                resource = new_resource;
                has_new = true;
                lock.unlock();
            }

            bool is_new() {
                return has_new;
            }

            /*!
             * \brief Copies the resource out of thie loaded_resource instance
             *
             * This method isn't amazing. It copies the resource (which could easily be something large) and returns the
             * copy. It returns by value, so there's a chance we'll get a cpoy elision but honestly who knows? I just
             * hope for the best
             *
             * \return A copy of the resource held by this thing
             */
            Resource get_resource() {
                /*
                 * I make a copy because I want the mutex to work as expected. If I didn't return a copy, and just
                 * returned the thing, the mutex would stay locked and we would have absolutely no party, so I copy the
                 * resource to a local variable so I can unlock the mutex before returning the thing
                 *
                 * This is probably a bad way to handle this situation. There's probable some super awesome trick
                 * involving dark magic and the alignment of the moone of Saturn, but this work so whatevs.
                 * loaded_resource::get_resource() shouldn't be called a lot, just when we're loading new data. That
                 * should happen pretty seldomly, so this shouldn't be a problem very often.
                 *
                 * If it is a problem, I'll deal with it then
                 */
                Resource copy;
                lock.lock();
                copy = resource;
                has_new = false;
                lock.unlock();

                return copy;
            }

        private:
            std::mutex lock;
            bool has_new;
            Resource resource;
        };
    }
}

#endif //RENDERER_LOADED_RESOURCE_H
