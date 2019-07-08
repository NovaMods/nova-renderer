#pragma once

#include "bvestl/polyalloc/bytes.hpp"

/*!
 * \brief Some useful utilities
 */

namespace bvestl {
	namespace polyalloc {
		constexpr EA_FORCE_INLINE Bytes align(const Bytes value, const Bytes alignment) noexcept {
			return ((value + (alignment - 1)) & ~(alignment - 1));
		}
	}
}
