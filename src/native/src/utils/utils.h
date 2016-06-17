/*!
 * \brief Contains a bunch of utility functions which may or may not be actually used anywhere
 *
 * \author David
 * \date 18-May-16.
 */

#ifndef RENDERER_UTILS_H
#define RENDERER_UTILS_H


/*!
 * \brief Initializes the logging system
 */
void initialize_logging();

/*!
 * \brief Casts the input thing into a unique_ptr
 *
 * This is mostly useful when you want a unique_ptr to an abstract class, and you want to initialize it with a child
 * class. That's how I use it, at least. Check out nova_renderer::nova_renderer to see an example useage
 *
 * This uses a static_cast. This means that you must be absolutely sure that PtrType is a parent class of InputType, or
 * else you're going to have a bad time
 */
template <typename PtrType, typename InputType>
std::unique_ptr<PtrType> make_unique(InputType * input) {
    return std::unique_ptr<PtrType>(static_cast<PtrType *>(input));
};

/*!
 * \brief Casts the input thing into a shared_ptr
 *
 * This is mostly useful when you want a shared_ptr to an abstract class, and you want to initialize it with a child
 * class. That's how I use it, at least.
 *
 * This uses a static_cast. This means that you must be absolutely sure that PtrType is a parent class of InputType, or
 * else you're going to have a bad time
 */
template <typename PtrType, typename InputType>
std::shared_ptr<PtrType> make_shared(InputType * input) {
    return std::shared_ptr<PtrType>(static_cast<PtrType *>(input));
};

#endif //RENDERER_UTILS_H
