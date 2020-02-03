#include "rx/core/types.h"
#include "rx/core/assert.h"
#include "rx/core/abort.h"

void* operator new(rx_size) {
  rx::abort("operator new is disabled");
}

void* operator new[](rx_size) {
  rx::abort("operator new[] is disabled");
}

void operator delete(void*) {
  rx::abort("operator delete is disabled");
}

void operator delete(void*, rx_size) {
  rx::abort("operator delete is disabled");
}

void operator delete[](void*) {
  rx::abort("operator delete[] is disabled");
}

void operator delete[](void*, rx_size) {
  rx::abort("operator delete[] is disabled");
}

extern "C" {
  void __cxa_pure_virtual() {
    rx::abort("pure virtual function call");
  }

  static constexpr rx_u8 k_complete{1 << 0};
  static constexpr rx_u8 k_pending{1 << 1};

  bool __cxa_guard_acquire(rx_u8* guard_) {
    if (guard_[1] == k_complete) {
      return false;
    }

    if (guard_[1] & k_pending) {
      rx::abort("recursive initialization unsupported");
    }

    guard_[1] = k_pending;
    return true;
  }

  void __cxa_guard_release(rx_u8* guard_) {
    guard_[1] = k_complete;
  }
}
