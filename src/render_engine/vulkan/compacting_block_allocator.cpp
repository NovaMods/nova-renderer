/*!
 * \author ddubois
 * \date 11-Nov-18.
 */

#include "compacting_block_allocator.hpp"
#include "../../util/logger.hpp"
#include "../render_engine.hpp"
#include "vulkan_utils.hpp"

namespace nova {
    uint32_t compacting_block_allocator::block_allocator_buffer::next_id = 0;

    compacting_block_allocator::block_allocator_buffer::block_allocator_buffer(const VkDeviceSize size, VmaAllocator allocator)
        : allocator(allocator), id(next_id++), size(size) {
        VmaAllocationCreateInfo allocate_info = {};
        allocate_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocate_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        NOVA_THROW_IF_VK_ERROR(vmaCreateBuffer(allocator, &buffer_info, &allocate_info, &buffer, &vma_allocation, &vma_allocation_info),
                               buffer_allocation_failed);

        // Setup the first block.
        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        head = new block_t();
        head->size = size;
        head->offset = 0;
        head->prev = nullptr;
        head->next = nullptr;
        head->free = true;
    }

    compacting_block_allocator::block_allocator_buffer::block_allocator_buffer(block_allocator_buffer&& other) noexcept
        : head(other.head), allocator(other.allocator), id(other.id), next_block_id(other.next_block_id), buffer(other.buffer),
          vma_allocation(other.vma_allocation), vma_allocation_info(other.vma_allocation_info), size(other.size),
          allocated(other.allocated) {
        other.buffer = VK_NULL_HANDLE;
    }

    compacting_block_allocator::block_allocator_buffer& compacting_block_allocator::block_allocator_buffer::operator=(
        block_allocator_buffer&& other) noexcept {
        head = other.head;
        id = other.id;
        next_block_id = other.next_block_id;
        buffer = other.buffer;
        vma_allocation = other.vma_allocation;
        vma_allocation_info = other.vma_allocation_info;
        size = other.size;
        allocated = other.allocated;
        allocator = other.allocator;

        other.buffer = VK_NULL_HANDLE;

        return *this;
    }

    compacting_block_allocator::block_allocator_buffer::~block_allocator_buffer() {
        if(buffer == VK_NULL_HANDLE) {
            return;
        }

        vmaDestroyBuffer(allocator, buffer, vma_allocation);

        block_t* prev = nullptr;
        block_t* current = head;
        while(true) {
            if(current->next == nullptr) {
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                delete current;
                break;
            }
            prev = current;
            current = current->next;
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete prev;
        }

        head = nullptr;
    }

    compacting_block_allocator::allocation_info* compacting_block_allocator::block_allocator_buffer::allocate(
        const VkDeviceSize needed_size) {
        return allocate_internal(needed_size, true);
    }

    void compacting_block_allocator::block_allocator_buffer::free(allocation_info* alloc) {
        block_t* current = nullptr;
        for(current = head; current != nullptr; current = current->next) {
            if(current->id == alloc->block_id) {
                break;
            }
        }

        if(current == nullptr) {
            NOVA_LOG(ERROR) << "compacting_block_allocator::block_allocator_buffer::free: Tried to free an unknown allocation. AllocatorL "
                            << this << " block ID: " << alloc->block_id;
            return;
        }

        current->free = true;

        if((current->prev != nullptr) && current->prev->free) {
            block_t* prev = current->prev;

            prev->next = current->next;
            if(current->next != nullptr) {
                current->next->prev = prev;
            }

            prev->size += current->size;

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete current;
            current = prev;
        }

        if((current->next != nullptr) && current->next->free) {
            block_t* next = current->next;

            if(next->next != nullptr) {
                next->next->prev = current;
            }

            current->next = next->next;

            current->size += next->size;

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            delete next;
        }

        allocated -= alloc->size;
    }

    VkBuffer compacting_block_allocator::block_allocator_buffer::get_buffer() const {
        return buffer;
    }

    compacting_block_allocator::allocation_info* compacting_block_allocator::block_allocator_buffer::allocate_internal(
        VkDeviceSize needed_size, bool can_compact) {
        const VkDeviceSize free_size = needed_size - allocated;
        if(free_size < needed_size) {
            return nullptr;
        }

        block_t* best_fit = nullptr;

        VkDeviceSize padding = 0;
        VkDeviceSize offset = 0;
        VkDeviceSize aligned_size = 0;

        for(block_t* current = head; current != nullptr; current = current->next) {
            if(!current->free) {
                continue;
            }

            if(needed_size > current->size) {
                continue;
            }

            offset = current->offset;

            padding = offset - current->offset;
            aligned_size = padding + needed_size;

            if(needed_size > current->size) {
                continue;
            }

            if(aligned_size + allocated >= size) {
                return nullptr;
            }

            best_fit = current;
            break;
        }

        if(best_fit == nullptr) {
            if(can_compact) {
                compact_all_memory();
                return allocate_internal(needed_size, false);
            }
            return nullptr;
        }

        if(best_fit->size > needed_size) {
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto* chunk = new block_t;
            block_t* next = best_fit->next;

            chunk->id = next_block_id++;
            chunk->prev = best_fit;
            best_fit->next = chunk;

            chunk->next = next;
            if(next != nullptr) {
                next->prev = chunk;
            }

            chunk->size = best_fit->size - aligned_size;
            chunk->offset = offset + needed_size;
            chunk->free = true;
        }

        best_fit->free = false;
        best_fit->size = needed_size;

        allocated += aligned_size;

        // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
        auto* allocation = new allocation_info;

        allocation->size = best_fit->size;
        allocation->block_id = best_fit->id;
        allocation->offset = offset;
        allocation->block = this;

        return allocation;
    }

    void compacting_block_allocator::block_allocator_buffer::compact_all_memory() {
        // Look for free blocks. If any are found, close them up

        void* write_ptr;
        vmaMapMemory(allocator, vma_allocation, &write_ptr);

        VkDeviceSize amount_compacted = 0;

        block_t* current = nullptr;
        for(current = head; current != nullptr;) {
            if(current->free && current->next != nullptr) {
                // Free block! Now we need to move things back

                // Don't read from memory that's free
                amount_compacted += current->size;
            }
            else {
                if(amount_compacted > 0) {
                    // The write and read pointers are different, which means that we need to move the current
                    // allocation back to fill the empty space

                    uint8_t* dst = reinterpret_cast<uint8_t*>(write_ptr) + amount_compacted;

                    std::memcpy(write_ptr, dst, current->size);
                    current->offset -= amount_compacted;
                }

                write_ptr = reinterpret_cast<uint8_t*>(write_ptr) + current->size;
            }
            if(current->next != nullptr) {
                current = current->next;
            }
            else {
                break;
            }
        }

        if(amount_compacted > 0) {
            // Create a free block at the end - after iterating through the list of blocks, we are at the end
            // e.g. current == end

            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto* const end_block = new block_t;
            end_block->id = next_block_id;
            next_block_id++;
            end_block->size = amount_compacted;
            end_block->offset = current->offset + current->size;
            end_block->prev = current;
            current->next = end_block;
        }
    }

    compacting_block_allocator::compacting_block_allocator(const settings_options::block_allocator_settings& settings,
                                                           VmaAllocator vma_allocator,
                                                           const uint32_t graphics_queue_idx,
                                                           const uint32_t copy_queue_idx)
        : settings(settings), vma_allocator(vma_allocator), graphics_queue_idx(graphics_queue_idx), copy_queue_idx(copy_queue_idx) {

        pools.emplace_back(block_allocator_buffer{settings.new_buffer_size, vma_allocator});
    }

    compacting_block_allocator::allocation_info* compacting_block_allocator::allocate(const VkDeviceSize size) {
        std::lock_guard l(pools_mutex);
        // First try to allocate from an existing pool
        for(block_allocator_buffer& buffer : pools) {
            allocation_info* allocation = buffer.allocate(size);
            if(allocation != nullptr) {
                return allocation;
            }
        }

        block_allocator_buffer new_buffer(settings.new_buffer_size, vma_allocator);
        allocation_info* allocation = new_buffer.allocate(size);
        pools.emplace_back(std::move(new_buffer));

        return allocation;
    }

    void compacting_block_allocator::free(allocation_info* allocation) {
        std::lock_guard l(pools_mutex);
        allocation->block->free(allocation);
    }

    void compacting_block_allocator::add_barriers_before_data_upload(VkCommandBuffer cmds) const {
        std::vector<VkBufferMemoryBarrier> barriers;
        barriers.reserve(pools.size());
        for(const block_allocator_buffer& pool : pools) {
            VkBufferMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.srcQueueFamilyIndex = graphics_queue_idx;
            barrier.dstQueueFamilyIndex = copy_queue_idx;
            barrier.buffer = pool.buffer;
            barrier.offset = 0;
            barrier.size = settings.new_buffer_size;

            barriers.push_back(barrier);
        }

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             static_cast<uint32_t>(pools.size()),
                             barriers.data(),
                             0,
                             nullptr);
    }

    void compacting_block_allocator::add_barriers_after_data_upload(VkCommandBuffer cmds) const {
        std::vector<VkBufferMemoryBarrier> barriers;
        barriers.reserve(pools.size());
        for(const block_allocator_buffer& pool : pools) {
            VkBufferMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
            barrier.srcQueueFamilyIndex = copy_queue_idx;
            barrier.dstQueueFamilyIndex = graphics_queue_idx;
            barrier.buffer = pool.buffer;
            barrier.offset = 0;
            barrier.size = settings.new_buffer_size;

            barriers.push_back(barrier);
        }

        vkCmdPipelineBarrier(cmds,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0,
                             nullptr,
                             static_cast<uint32_t>(pools.size()),
                             barriers.data(),
                             0,
                             nullptr);
    }
} // namespace nova
