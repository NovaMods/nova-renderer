#ifndef RX_CORE_MEMORY_UNINITIALIZED_STORAGE_H
#define RX_CORE_MEMORY_UNINITIALIZED_STORAGE_H
#include "rx/core/utility/construct.h"
#include "rx/core/utility/destruct.h"
#include "rx/core/utility/forward.h"
#include "rx/core/utility/nat.h"

#include "rx/core/concepts/no_copy.h"
#include "rx/core/concepts/no_move.h"

#include "rx/core/hints/empty_bases.h"

namespace rx::memory {

// represents uninitialized storage suitable in size and alignment for
// an object of type |T|, can be type erased for implementing deferred static
// globals and variant types
template<typename T>
struct RX_HINT_EMPTY_BASES uninitialized_storage
  : concepts::no_copy
  , concepts::no_move
{
  constexpr uninitialized_storage();

  // explicitly initialize the storage with |args|
  template<typename... Ts>
  void init(Ts&&... _args);

  // explicitly finalize the storage
  void fini();

  // get the storage
  T* data();
  const T* data() const;

private:
  union {
    utility::nat m_nat;
    alignas(T) mutable rx_byte m_data[sizeof(T)];
  };
};

// uninitialized_storage
template<typename T>
inline constexpr uninitialized_storage<T>::uninitialized_storage()
  : m_nat{}
{
}

template<typename T>
template<typename... Ts>
inline void uninitialized_storage<T>::init(Ts&&... _args) {
  utility::construct<T>(reinterpret_cast<void*>(m_data), utility::forward<Ts>(_args)...);
}

template<typename T>
inline void uninitialized_storage<T>::fini() {
  utility::destruct<T>(reinterpret_cast<void*>(m_data));
}

template<typename T>
inline T* uninitialized_storage<T>::data() {
  return reinterpret_cast<T*>(m_data);
}

template<typename T>
inline const T* uninitialized_storage<T>::data() const {
  return reinterpret_cast<const T*>(m_data);
}

} // namespace rx::memory

#endif // RX_CORE_MEMORY_UNINITIALIZED_STORAGE_H
