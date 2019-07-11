#pragma once

#include "nova_renderer/bytes.hpp"

namespace bvestl {
	namespace polyalloc {
        struct NOVA_API AllocationInfo;

        /*!
         * \brief Interface for an allocation strategy
         */
        class NOVA_API AllocationStrategy {
		public:
			virtual ~AllocationStrategy() = default;

			virtual bool allocate(Bytes size, AllocationInfo& allocation) = 0;

			virtual void free(const AllocationInfo& alloc) = 0;
		};
	}
}
