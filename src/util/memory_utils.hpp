#pragma once

#include "nova_renderer/bytes.hpp"

/*!
 * \brief Some useful utilities
 */

namespace bvestl {
	namespace polyalloc {
		constexpr Bytes align(const Bytes value, const Bytes alignment) noexcept {
			return ((value + (alignment - 1)) & ~(alignment - 1));
		}
	}
}
