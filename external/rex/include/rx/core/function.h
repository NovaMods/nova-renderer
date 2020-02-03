#ifndef RX_CORE_FUNCTION_H
#define RX_CORE_FUNCTION_H
#include "rx/core/vector.h"

#include "rx/core/traits/is_callable.h"
#include "rx/core/traits/enable_if.h"

namespace rx {

template<typename T>
struct function;

template<typename R, typename... Ts>
struct function<R(Ts...)> {
  constexpr function(memory::allocator* _allocator);
  constexpr function();

  template<typename F, typename = traits::enable_if<traits::is_callable<F, Ts...>>>
  function(memory::allocator* _allocator, F&& _function);

  template<typename F, typename = traits::enable_if<traits::is_callable<F, Ts...>>>
  function(F&& _function);

  function(const function& _function);
  function(function&& function_);

  function& operator=(const function& _function);
  function& operator=(function&& function_);
  function& operator=(rx_nullptr);

  ~function();

  R operator()(Ts... _arguments);

  operator bool() const;

  memory::allocator* allocator() const;

private:
  using invoke_fn = R (*)(rx_byte*, Ts&&...);
  using construct_fn = void (*)(rx_byte*, const rx_byte*);
  using destruct_fn = void (*)(rx_byte*);

  template<typename F>
  static R invoke(rx_byte* _function, Ts&&... _arguments) {
    if constexpr(traits::is_same<R, void>) {
      (*reinterpret_cast<F*>(_function))(utility::forward<Ts>(_arguments)...);
    } else {
      return (*reinterpret_cast<F*>(_function))(utility::forward<Ts>(_arguments)...);
    }
  }

  template<typename F>
  static void construct(rx_byte* _dst, const rx_byte* _src) {
    utility::construct<F>(_dst, *reinterpret_cast<const F*>(_src));
  }

  template<typename F>
  static void destruct(rx_byte* _function) {
    utility::destruct<F>(_function);
  }

  invoke_fn m_invoke;
  construct_fn m_construct;
  destruct_fn m_destruct;
  vector<rx_byte> m_data;
};

template<typename R, typename... Ts>
inline constexpr function<R(Ts...)>::function(memory::allocator* _allocator)
  : m_invoke{nullptr}
  , m_construct{nullptr}
  , m_destruct{nullptr}
  , m_data{_allocator}
{
}

template<typename R, typename... Ts>
inline constexpr function<R(Ts...)>::function()
  : function{&memory::g_system_allocator}
{
}

template<typename R, typename... Ts>
template<typename F, typename>
inline function<R(Ts...)>::function(F&& _function)
  : function{&memory::g_system_allocator, utility::forward<F>(_function)}
{
}

template<typename R, typename... Ts>
template<typename F, typename>
inline function<R(Ts...)>::function(memory::allocator* _allocator, F&& _function)
  : m_invoke{invoke<F>}
  , m_construct{construct<F>}
  , m_destruct{destruct<F>}
  , m_data{_allocator}
{
  m_data.resize(sizeof _function, utility::uninitialized{});
  m_construct(m_data.data(), reinterpret_cast<rx_byte*>(&_function));
}

template<typename R, typename... Ts>
inline function<R(Ts...)>::function(const function& _function)
  : m_invoke{_function.m_invoke}
  , m_construct{_function.m_construct}
  , m_destruct{_function.m_destruct}
{
  if (m_invoke) {
    m_data.resize(_function.m_data.size(), utility::uninitialized{});
    m_construct(m_data.data(), _function.m_data.data());
  }
}

template<typename R, typename... Ts>
inline function<R(Ts...)>::function(function&& function_)
  : m_invoke{function_.m_invoke}
  , m_construct{function_.m_construct}
  , m_destruct{function_.m_destruct}
  , m_data{utility::move(function_.m_data)}
{
  function_.m_invoke = nullptr;
  function_.m_construct = nullptr;
  function_.m_destruct = nullptr;
}

template<typename R, typename... Ts>
inline function<R(Ts...)>& function<R(Ts...)>::operator=(const function& _function) {
  RX_ASSERT(&_function != this, "self assignment");

  if (m_destruct) {
    m_destruct(m_data.data());
  }

  m_invoke = _function.m_invoke;
  m_construct = _function.m_construct;
  m_destruct = _function.m_destruct;

  if (m_invoke) {
    m_data.resize(_function.m_data.size(), utility::uninitialized{});
    m_construct(m_data.data(), _function.m_data.data());
  }

  return *this;
}

template<typename R, typename... Ts>
inline function<R(Ts...)>& function<R(Ts...)>::operator=(function&& function_) {
  RX_ASSERT(&function_ != this, "self assignment");

  if (m_destruct) {
    m_destruct(m_data.data());
  }

  m_invoke = function_.m_invoke;
  m_construct = function_.m_construct;
  m_destruct = function_.m_destruct;
  m_data = utility::move(function_.m_data);

  function_.m_invoke = nullptr;
  function_.m_construct = nullptr;
  function_.m_destruct = nullptr;

  return *this;
}

template<typename R, typename... Ts>
inline function<R(Ts...)>& function<R(Ts...)>::operator=(rx_nullptr) {
  if (m_destruct) {
    m_destruct(m_data.data());
  }

  m_invoke = nullptr;
  m_construct = nullptr;
  m_destruct = nullptr;

  return *this;
}

template<typename R, typename... Ts>
inline function<R(Ts...)>::~function() {
  if (m_destruct) {
    m_destruct(m_data.data());
  }
}

template<typename R, typename... Ts>
inline R function<R(Ts...)>::operator()(Ts... _arguments) {
  if constexpr(traits::is_same<R, void>) {
    m_invoke(m_data.data(), utility::forward<Ts>(_arguments)...);
  } else {
    return m_invoke(m_data.data(), utility::forward<Ts>(_arguments)...);
  }
}

template<typename R, typename... Ts>
function<R(Ts...)>::operator bool() const {
  return m_invoke != nullptr;
}

template<typename R, typename... Ts>
inline memory::allocator* function<R(Ts...)>::allocator() const {
  return m_data.allocator();
}

} // namespace rx::core

#endif // RX_CORE_FUNCTION_H
