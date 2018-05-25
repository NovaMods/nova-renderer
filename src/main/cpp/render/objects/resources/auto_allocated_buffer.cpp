/*!
 * \author ddubois 
 * \date 12-Feb-18.
 */

#include <sstream>
#include <easylogging++.h>
#include "auto_allocated_buffer.h"

namespace nova {
    auto_buffer::auto_buffer(std::string name, std::shared_ptr<render_context> context, vk::BufferCreateInfo create_info, uint64_t min_alloc_size, bool mapped = false) :
            uniform_buffer(name, context, create_info, min_alloc_size, mapped) {

        chunks.emplace_back(auto_buffer_chunk{vk::DeviceSize(0), create_info.size});
    }

    vk::DescriptorBufferInfo auto_buffer::allocate_space(uint64_t size) {
        size = size > min_alloc_size ? size : min_alloc_size;
        int32_t index_to_allocate_from = -1;
        if(!chunks.empty()) {
            // Iterate backwards so that inserting or deleting has a minimal cost
            for(auto i = static_cast<int32_t>(chunks.size() - 1); i >= 0; --i) {
                LOG(TRACE) << "Checking if chunk " << i << "'s range, " << chunks[i].range << " is greater than or equal to " << size;
                if(chunks[i].range >= size) {
                    LOG(TRACE) << "It is!";
                    index_to_allocate_from = i;
                }
            }
        }

        if(index_to_allocate_from == -1) {
            // Whoops, couldn't find anything
            auto ss = std::stringstream{};
            ss << "No big enough slots in the buffer. There's " << chunks.size() << " slots. If there's a lot then you got some fragmentation";

            LOG(ERROR) << "No big enough slots in the buffer. There's " << chunks.size() << " slots. If there's a lot then you got some fragmentation";
            // Halt execution like a boss
            throw std::runtime_error(ss.str());
        }

        auto& chunk_to_allocate_from = chunks[index_to_allocate_from];
        if(chunk_to_allocate_from.range == size) {
            // Easy: unallocate the chunk, return the chunks

            auto ret_val = vk::DescriptorBufferInfo{buffer, chunk_to_allocate_from.offset, chunk_to_allocate_from.range};
            chunks.erase(chunks.begin() + index_to_allocate_from);
            return ret_val;
        }

        // The chunk is bigger than we need. Allocate at the beginning of it so our iteration algorithm finds the next
        // free chunk nice and early, then shrink it

        auto ret_val = vk::DescriptorBufferInfo{buffer, chunk_to_allocate_from.offset, size};

        chunk_to_allocate_from.offset += size;
        chunk_to_allocate_from.range -= size;

        return ret_val;
    }

    void auto_buffer::free_allocation(const vk::DescriptorBufferInfo& to_free) {
        // This one will be hard...
        // Properly we should try to find an allocated space of our size and merge the two allocations on either side of
        // it... but that's marginally harder (maybe I'll do it) so let's just try to find the first slot it will fit

        LOG(DEBUG) << "Freeing allocation offset=" << to_free.offset << " range=" << to_free.range;

        auto to_free_end = to_free.offset + to_free.range;

        auto& first_chunk = chunks[0];
        auto& last_chunk = chunks[chunks.size() - 1];

        if(chunks.empty()) {
            chunks.emplace_back(auto_buffer_chunk{to_free.offset, to_free.range});
            LOG(TRACE) << "No available chunks, so we added the old allocation wholesale";
            goto end;
        }

        if(last_chunk.offset + last_chunk.range == to_free.offset) {
            last_chunk.range += to_free.range;
            LOG(TRACE) << "Expanded the last chunk to cover the newly freed allocation";
            goto end;
        }

        if(last_chunk.offset + last_chunk.range < to_free.offset) {
            chunks.emplace_back(auto_buffer_chunk{to_free.offset, to_free.range});
            LOG(TRACE) << "Inserted a new chunk at the very end of the list";
            goto end;
        }

        if(to_free_end == first_chunk.offset) {
            first_chunk.offset -= to_free.offset;
            first_chunk.range += to_free.range;
            LOG(TRACE) << "Expanded the first chunk backwards to cover the freed allocation";
            goto end;
        }

        if(to_free_end < first_chunk.offset) {
            chunks.emplace(chunks.begin(), auto_buffer_chunk{to_free.offset, to_free.range});
            LOG(TRACE) << "Inserting a new chunk at the beginning of the list";
            goto end;
        }

        for(auto i = chunks.size() - 1; i >= 1; i++) {
            auto& behind_space = chunks[i - 1];
            auto& ahead_space = chunks[i];

            auto behind_space_end = behind_space.offset + behind_space.range;
            auto space_between_allocs = space_between(behind_space, ahead_space);

            // Do we fit nicely between the two things?
            if(space_between_allocs == to_free.range) {
                // combine these nerds
                chunks[i - 1].range += to_free.range + ahead_space.range;
                chunks.erase(chunks.begin() + i);
                LOG(TRACE) << "Freed chunk fits between two existing chunks, so the previous one was expanded and the next one was erased";
                goto end;
            }

            // Do we fit up against one of the two things?
            if(space_between_allocs > to_free.range) {
                if(behind_space_end == to_free.offset) {
                    chunks[i - 1].range += to_free.range;
                    LOG(TRACE) << "Expanded chunk " << i - 1 << " forward for the freed allocation";
                    goto end;
                }

                if(to_free_end == ahead_space.offset) {
                    chunks[i].offset -= to_free.range;
                    chunks[i].range += to_free.range;
                    LOG(TRACE) << "Expanded chunk " << i << " backwards for the freed allocation";
                    goto end;
                }

                chunks.emplace(chunks.begin() + i, auto_buffer_chunk{to_free.offset, to_free.range});
                LOG(TRACE) << "Just made a new chunk cause it's not next to an existing chunk";
                goto end;
            }
        }

        // We got here... without returning our allocation to the pool. Uhm... Did we double-allocate something? This is
        // a bug in my allocator and not something that should happen during Nova so let's just crash
        LOG(FATAL) << "Could not return allocation {offset=" << to_free.offset << " range=" << to_free.range << "} which should not happen. There's probably a bug in the allocator and you need to debug it";
        end:

        LOG(TRACE) << "Current chunks:";

        auto ss = std::stringstream{};
        for(const auto& chunk : chunks) {
            ss << "{offset=" << (uint64_t)chunk.offset << " range=" << (uint64_t)chunk.range << "} ";
        }
        LOG(TRACE) << ss.str();
    }
	
    vk::DeviceSize space_between(const auto_buffer_chunk& first, const auto_buffer_chunk& last) {
        return last.offset - (first.offset + first.range);
    }
}
