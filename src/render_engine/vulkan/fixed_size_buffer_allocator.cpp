#include "fixed_size_buffer_allocator.hpp"

namespace nova::renderer {
    template <uint32_t BlockSize>
    fixed_size_buffer_allocator<BlockSize>::fixed_size_buffer_allocator(
        const std::string& name, VmaAllocator allocator, const VkBufferCreateInfo& create_info, const uint64_t alignment, const bool mapped)
        : uniform_buffer(name, allocator, create_info, alignment, mapped), num_blocks(create_info.size / BlockSize) {
        blocks = new block[num_blocks];
        for(uint32_t i = 0; i < num_blocks; i++) {
            blocks[i].index = i;
            if(i < num_blocks - 1) {
                blocks[i].next = &blocks[i + 1];
            }
            else {
                blocks[i].next = nullptr;
            }
        }
    }

    template <uint32_t BlockSize>
    fixed_size_buffer_allocator<BlockSize>::fixed_size_buffer_allocator(fixed_size_buffer_allocator&& old) noexcept
        : uniform_buffer(std::forward(old)), blocks(old.blocks), num_blocks(old.num_blocks) {
        old.blocks = nullptr;
    }

    template <uint32_t BlockSize>
    fixed_size_buffer_allocator<BlockSize>& fixed_size_buffer_allocator<BlockSize>::operator=(fixed_size_buffer_allocator&& old) noexcept {
        uniform_buffer::operator=(std::forward<uniform_buffer>(old));
        blocks = old.blocks;
        num_blocks = old.num_blocks;
        old.blocks = nullptr;

        return *this;
    }

    template <uint32_t BlockSize>
    fixed_size_buffer_allocator<BlockSize>::~fixed_size_buffer_allocator() {
        if(blocks != nullptr) {

            // Find the first block and issue the `delete[]` on that
            for(uint32_t i = 0; i < num_blocks; i++) {
                if(blocks[i].index == 0) {
                    delete[] & blocks[i];

                    break;
                }
            }
        }
    }

    template <uint32_t BlockSize>
    typename fixed_size_buffer_allocator<BlockSize>::block* fixed_size_buffer_allocator<BlockSize>::allocate_block() {
        block* ret_val = blocks;
        blocks = blocks->next;

        return ret_val;
    }

    template <uint32_t BlockSize>
    void fixed_size_buffer_allocator<BlockSize>::free_block(block* freed_block) {
        freed_block->next = blocks;
        blocks = freed_block;
    }
} // namespace nova::renderer
