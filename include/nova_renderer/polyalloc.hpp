#pragma once

#include <cstdlib>
#include <EABase/config/eacompilertraits.h>
#include <EASTL/internal/config.h>

#ifndef BVESTL_POLYALLOC_ASSERT
	#include <cassert>
	#define BVESTL_POLYALLOC_ASSERT assert
#endif

#ifndef BVESTL_POLYALLOC_NO_ASSERT_ON_DEFAULT_CONSTRUCT
	#define BVESTL_POLYALLOC_DEFAULT_CONSTRUCT_ASSERT BVESTL_POLYALLOC_ASSERT(false && "Default Constructing an AllocatorHandle is always a bug.")
#else
	#define BVESTL_POLYALLOC_DEFAULT_CONSTRUCT_ASSERT
#endif

namespace bvestl {
	namespace polyalloc {
		class Allocator {
		  public:
			Allocator() = default;
			Allocator(Allocator const&) = delete;
			Allocator(Allocator&&) = delete;
			Allocator& operator=(Allocator const&) = delete;
			Allocator& operator=(Allocator&&) = delete;
			
			virtual ~Allocator() = default;

			virtual void* allocate(size_t n, int flags = 0) = 0;
			virtual void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0) = 0;
			virtual void deallocate(void* p, size_t n) = 0;
		};

		class allocator_handle {
		  public:
		  	// Needs to exist because EASTL won't compile without it. Should not ever be called.
			EA_FORCE_INLINE allocator_handle(char const* const = nullptr) noexcept : allocator_(nullptr) {
				BVESTL_POLYALLOC_DEFAULT_CONSTRUCT_ASSERT;
			}

			EA_FORCE_INLINE allocator_handle(Allocator* const allocator, char const* const = nullptr) noexcept : allocator_(allocator) {}

			EA_FORCE_INLINE allocator_handle(const allocator_handle& x, char const* const = nullptr) noexcept : allocator_(x.allocator_) {}

			EA_FORCE_INLINE ~allocator_handle() noexcept = default;

			EA_FORCE_INLINE allocator_handle& operator=(const allocator_handle& x) noexcept = default;

			EA_FORCE_INLINE void* allocate(size_t const n, int const flags = 0) {
				return allocator_->allocate(n, flags);
			}

			EA_FORCE_INLINE void* allocate(size_t const n, size_t const alignment, size_t const offset, int const flags = 0) {
				return allocator_->allocate(n, alignment, offset, flags);
			}

			EA_FORCE_INLINE void deallocate(void* const p, size_t const n) {
				allocator_->deallocate(p, n);
			}

			EA_FORCE_INLINE char const* get_name() const noexcept {
				return "EASTL Polyalloc Allocator Handle";
			}

			EA_FORCE_INLINE void set_name(char const* const) const noexcept {}

			EA_FORCE_INLINE bool operator==(allocator_handle const& rhs) const {
				return allocator_ == rhs.allocator_;
			}

			EA_FORCE_INLINE bool operator!=(allocator_handle const& rhs) const {
				return !(rhs == *this);
			}

		  private:
			Allocator* allocator_;
		};
	}
}
