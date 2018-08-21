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
    struct per_thread_datapp {
        // Tasks can allocate, record, and execute command buffers, so each thread needs its own command buffer pool
        command_pool cmd_buf_pool;
    };
}

#endif //NOVA_RENDERER_THREADING_CONTEXT_H
