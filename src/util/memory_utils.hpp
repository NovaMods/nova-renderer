#pragma once

#include "nova_renderer/bytes.hpp"

/*!
 * \brief Some useful utilities
 */

namespace bvestl {
	namespace polyalloc {
		constexpr Bytes align(const Bytes value, const Bytes alignment) noexcept {
		    // TODO: Make faster
			return alignment == Bytes(0) ? value :
			    (value % alignment == Bytes(0) ? value : value + value % alignment);
		}
	}
}
