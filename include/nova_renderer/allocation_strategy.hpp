#pragma once

#include "nova_renderer/bytes.hpp"

namespace bvestl {
	namespace polyalloc {
		struct AllocationInfo;

        /*!
         * \brief Interface for an allocation strategy
         */
		class AllocationStrategy {
		public:
			virtual ~AllocationStrategy() = default;

			virtual bool allocate(Bytes size, AllocationInfo& allocation) = 0;

			virtual void free(const AllocationInfo& alloc) = 0;
		};
	}
}
