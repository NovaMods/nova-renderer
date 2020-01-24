#ifndef RX_CORE_EVENT_H
#define RX_CORE_EVENT_H
#include "rx/core/function.h"

namespace rx {

template<typename... Ts>
struct event {
  using delegate = function<void(Ts...)>;

  struct handle
    : concepts::no_copy
  {
    constexpr handle(event* _event, rx_size _index);
    constexpr handle(handle&& _existing);
    ~handle();
  private:
    event* m_event;
    rx_size m_index;
  };

  constexpr event(memory::allocator* _allocator);
  constexpr event();

  void signal(Ts... _arguments);
  handle connect(delegate&& function_);

  memory::allocator* allocator() const;

private:
  friend struct handle;

  vector<delegate> m_delegates;
};

template<typename... Ts>
inline constexpr event<Ts...>::handle::handle(event<Ts...>* _event, rx_size _index)
  : m_event{_event}
  , m_index{_index}
{
}

template<typename... Ts>
inline constexpr event<Ts...>::handle::handle(handle&& handle_)
  : m_event{handle_.m_event}
  , m_index{handle_.m_index}
{
  handle_.m_event = nullptr;
  handle_.m_index = 0;
}

template<typename... Ts>
inline event<Ts...>::handle::~handle() {
  if (m_event) {
    m_event->m_delegates[m_index] = nullptr;
  }
}

template<typename... Ts>
inline constexpr event<Ts...>::event(memory::allocator* _allocator)
  : m_delegates{_allocator}
{
}

template<typename... Ts>
inline constexpr event<Ts...>::event()
  : event{&memory::g_system_allocator}
{
}

template<typename... Ts>
inline void event<Ts...>::signal(Ts... _arguments) {
  m_delegates.each_fwd([&](delegate& _delegate) {
    if (_delegate) {
      _delegate(_arguments...);
    }
  });
}

template<typename... Ts>
inline typename event<Ts...>::handle event<Ts...>::connect(delegate&& delegate_) {
  const rx_size delegates{m_delegates.size()};
  for (rx_size i{0}; i < delegates; i++) {
    if (!m_delegates[i]) {
      return {this, i};
    }
  }
  m_delegates.emplace_back(utility::move(delegate_));
  return {this, delegates};
}

template<typename... Ts>
inline memory::allocator* event<Ts...>::allocator() const {
  return m_delegates.allocator();
}

} // namespace rx

#endif // RX_CORE_EVENT_H
