/*!
 * \author ddubois 
 * \date 17-Aug-18.
 */

#ifndef NOVA_RENDERER_THREADING_CONTEXT_H
#define NOVA_RENDERER_THREADING_CONTEXT_H

namespace nova {
    /*!
     * \brief Per-thread data
     */
    class per_thread_data {
    private:
        command_pool pool;
    };
}

#endif //NOVA_RENDERER_THREADING_CONTEXT_H
