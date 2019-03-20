#pragma once
#include "cached_buffer.hpp"
#include "../../util/logger.hpp"

namespace nova::renderer {
    /*!
     * \brief A uniform buffer which holds a single struct
     * 
     * \tparam DataType The type of the struct that this UBO holds
     */
    template <typename DataType>
    class struct_uniform_buffer : public cached_buffer {
    public:
        struct_uniform_buffer(std::string name, VkDevice device, VmaAllocator allocator, VkBufferCreateInfo& create_info, uint64_t alignment) : cached_buffer(name, device, allocator, create_info, alignment) {
            // Would love to set create_info.size to sizeof(DataType) before calling uniform_buffer constructor, but C++ doesn't allow for that :(
            if(create_info.size != sizeof(DataType)) {
                NOVA_LOG(ERROR) << "create_info.size is " << create_info.size << " but sizeof(DataType) is " << sizeof(DataType) << ". They must be the same!";
            }
        }
    };
}
