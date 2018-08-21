/*!
 * \author ddubois 
 * \date 19-Aug-18.
 */

#ifndef NOVA_RENDERER_TIMESTAMP_QUERY_POOL_H
#define NOVA_RENDERER_TIMESTAMP_QUERY_POOL_H

#include <vulkan/vulkan.hpp>

namespace nova {
    /*!
     * \brief Holds information about the timestamp query pool
     */
    class timestamp_query_pool {
    public:
        void recreate_timestamp_query_pool(uint32_t size);

    private:
        uint32_t timestamp_valid_bits;
        float timestamp_period;
        vk::QueryPool query_pool;
    };
}

#endif //NOVA_RENDERER_TIMESTAMP_QUERY_POOL_H
