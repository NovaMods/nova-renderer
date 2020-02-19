#ifndef RX_CORE_EVENT_H
#define RX_CORE_EVENT_H
#include "rx/core/function.h"

#include "rx/core/concurrency/spin_lock.h"
#include "rx/core/concurrency/scope_lock.h"

#include "rx/core/hints/empty_bases.h"

namespace rx {

template<typename T>
struct event;

template<typename R, typename... Ts>
struct event<R(Ts...)> {
  using delegate = function<R(Ts...)>;

  struct RX_HINT_EMPTY_BASES handle
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

  rx_size size() const;
  bool is_empty() const;

  memory::allocator* allocator() const;

private:
  friend struct handle;
  mutable concurrency::spin_lock m_lock;
  vector<delegate> m_delegates; // protected by |m_lock|
};

template<typename R, typename... Ts>
inline constexpr event<R(Ts...)>::handle::handle(event<R(Ts...)>* _event, rx_size _index)
  : m_event{_event}
  , m_index{_index}
{
}

template<typename R, typename... Ts>
inline constexpr event<R(Ts...)>::handle::handle(handle&& handle_)
  : m_event{handle_.m_event}
  , m_index{handle_.m_index}
{
  handle_.m_event = nullptr;
  handle_.m_index = 0;
}

template<typename R, typename... Ts>
inline event<R(Ts...)>::handle::~handle() {
  if (m_event) {
    concurrency::scope_lock lock{m_event->m_lock};
    m_event->m_delegates[m_index] = nullptr;
  }
}

template<typename R, typename... Ts>
inline constexpr event<R(Ts...)>::event(memory::allocator* _allocator)
  : m_delegates{_allocator}
{
}

template<typename R, typename... Ts>
inline constexpr event<R(Ts...)>::event()
  : event{&memory::g_system_allocator}
{
}

template<typename R, typename... Ts>
inline void event<R(Ts...)>::signal(Ts... _arguments) {
  concurrency::scope_lock lock{m_lock};
  m_delegates.each_fwd([&](delegate& _delegate) {
    if (_delegate) {
      _delegate(_arguments...);
    }
  });
}

template<typename R, typename... Ts>
inline typename event<R(Ts...)>::handle event<R(Ts...)>::connect(delegate&& delegate_) {
  concurrency::scope_lock lock{m_lock};
  const rx_size delegates{m_delegates.size()};
  for (rx_size i{0}; i < delegates; i++) {
    if (!m_delegates[i]) {
      return {this, i};
    }
  }
  m_delegates.emplace_back(utility::move(delegate_));
  return {this, delegates};
}

template<typename R, typename... Ts>
inline bool event<R(Ts...)>::is_empty() const {
  return size() == 0;
}

template<typename R, typename... Ts>
inline rx_size event<R(Ts...)>::size() const {
  concurrency::scope_lock lock{m_lock};
  // This is slightly annoying because |m_delegates| may have empty slots.
  rx_size result = 0;
  m_delegates.each_fwd([&](const delegate& _delegate) {
    if (_delegate) {
      result++;
    }
  });
  return result;
}

template<typename R, typename... Ts>
inline memory::allocator* event<R(Ts...)>::allocator() const {
  return m_delegates.allocator();
}

} // namespace rx

#endif // RX_CORE_EVENT_H
