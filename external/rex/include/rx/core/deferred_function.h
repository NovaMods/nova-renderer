#ifndef RX_CORE_DEFERRED_FUNCTION_H
#define RX_CORE_DEFERRED_FUNCTION_H
#include "rx/core/function.h"

namespace rx {

// callable function that gets called when the object goes out of scope
template<typename T>
struct deferred_function {
  template<typename F>
  constexpr deferred_function(memory::allocator* _allocator, F&& _function);

  template<typename F>
  constexpr deferred_function(F&& _function);

  ~deferred_function();

  memory::allocator* allocator() const;

private:
  function<T> m_function;
};

template<typename T>
template<typename F>
inline constexpr deferred_function<T>::deferred_function(memory::allocator* _allocator, F&& _function)
  : m_function{_allocator, utility::forward<F>(_function)}
{
}

template<typename T>
template<typename F>
inline constexpr deferred_function<T>::deferred_function(F&& _function)
  : deferred_function{&memory::g_system_allocator, utility::forward<F>(_function)}
{
}

template<typename T>
inline deferred_function<T>::~deferred_function() {
  m_function();
}

template<typename T>
inline memory::allocator* deferred_function<T>::allocator() const {
  return m_function.allocator();
}

} // namespace rx

#endif // RX_CORE_DEFERRED_FUNCTION_H
