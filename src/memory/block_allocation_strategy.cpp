#include "block_allocation_strategy.hpp"
#include "nova_renderer/polyalloc.hpp"
#include "nova_renderer/allocation_structs.hpp"
#include "../util/memory_utils.hpp"

using namespace bvestl::polyalloc::operators;

namespace bvestl {
	namespace polyalloc {
		BlockAllocationStrategy::BlockAllocationStrategy(allocator_handle& allocator_in, const Bytes size, const Bytes alignment_in)
			: allocator(allocator_in), memory_size(size), alignment(alignment_in) {

			head = make_new_block(0_b, size);
		}

		BlockAllocationStrategy::~BlockAllocationStrategy() {
			Block* next = nullptr;
			Block* current = head;

			while (current) {
				if (current->next) {
					next = current->next;
				}

				allocator.deallocate(current, sizeof(Block));

				current = next;
			}
		}

		bool BlockAllocationStrategy::allocate(Bytes size, AllocationInfo& allocation) {
			size = align(size, alignment);

			const Bytes free_size = memory_size - allocated;
			if (free_size < size) {
				return false;
			}

			Block* best_fit = nullptr;

			for (Block* current = head; current; current = current->next) {
				if (!current->free || current->size < size) {
					continue;
				}

				if (best_fit == nullptr) {
					best_fit = current;
					break;
				}
			}

			if (!best_fit) {
				return false;
			}

			if (best_fit->size > size) {
				Block* block = make_new_block(best_fit->offset + size, best_fit->size - size);

				best_fit->next = block;
				block->previous = best_fit;
				block->next = best_fit->next;

				best_fit = block;
			}

			allocated += size;

			allocation.size = size;
			allocation.offset = best_fit->offset;
			allocation.internal_data = best_fit;

			return true;
		}

		void BlockAllocationStrategy::free(const AllocationInfo& alloc) {
			Block* block = static_cast<Block*>(alloc.internal_data);
			block->free = true;

			if (block->previous && block->previous->free) {
				// We aren't the first block in the list, and the previous block is free. Merge this block into the previous block
				Block* prev = block->previous;

				prev->next = block->next;
				if (block->next) {
					block->next->previous = prev;
				}

				prev->size += block->size;

				allocator.deallocate(block, sizeof(Block));

				block = prev;
			}

			if (block->next && block->next->free) {
				// There's a block right after us in the list, and it's free. Merge the next block into this block
				Block* next = block->next;

				if (next->next) {
					next->next->previous = block;
				}

				block->next = next->next;
				block->size += next->size;

				allocator.deallocate(next, sizeof(Block));
			}

			allocated -= alloc.size;
		}

		BlockAllocationStrategy::Block* BlockAllocationStrategy::make_new_block(const Bytes offset, const Bytes size) {
			void* mem = allocator.allocate(sizeof(Block));
			Block* block = new(mem) Block;
			block->id = next_block_id;
			block->size = size;
			block->offset = offset;

			next_block_id++;

			return block;
		}
	}
}
