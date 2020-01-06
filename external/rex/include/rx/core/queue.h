#ifndef RX_CORE_QUEUE_H
#define RX_CORE_QUEUE_H
#include "rx/core/utility/move.h"
#include "rx/core/utility/forward.h"

#include "rx/core/memory/system_allocator.h"

#include "rx/core/assert.h"

namespace rx {

template<typename T>
struct queue {
  constexpr queue(memory::allocator* _allocator);
  constexpr queue();

  ~queue();
  bool is_empty() const;

  void push(const T& _value);

  template<typename... Ts>
  void emplace(Ts&&... _arguments);

  T pop();

  void clear();

  memory::allocator* allocator() const;

private:
  struct node {
    constexpr node(const T& _value);
    constexpr node(T&& value_);
    T m_value;
    node* m_next;
  };

  memory::allocator* m_allocator;
  node* m_first;
  node* m_last;
};

template<typename T>
inline constexpr queue<T>::node::node(const T& _value)
  : m_value{_value}
  , m_next{nullptr}
{
}

template<typename T>
inline constexpr queue<T>::node::node(T&& value_)
  : m_value{utility::move(value_)}
  , m_next{nullptr}
{
}

template<typename T>
inline constexpr queue<T>::queue(memory::allocator* _allocator)
  : m_allocator{_allocator}
  , m_first{nullptr}
  , m_last{nullptr}
{
}

template<typename T>
inline constexpr queue<T>::queue()
  : queue{&memory::g_system_allocator}
{
}

template<typename T>
inline queue<T>::~queue() {
  clear();
}

template<typename T>
inline bool queue<T>::is_empty() const {
  return !m_first;
}

template<typename T>
inline void queue<T>::push(const T& _value) {
  auto new_node{m_allocator->create<node>(_value)};
  if (is_empty()) {
    m_first = new_node;
    m_last = m_first;
  } else {
    auto last_node{m_last};
    last_node->m_next = new_node;
    m_last = last_node->m_next;
  }
}

template<typename T>
template<typename... Ts>
inline void queue<T>::emplace(Ts&&... _arguments) {
  auto new_node{m_allocator->create<node>(utility::forward<Ts>(_arguments)...)};
  if (is_empty()) {
    m_first = new_node;
    m_last = m_first;
  } else {
    auto last_node{m_last};
    last_node->m_next = new_node;
    m_last = last_node->m_next;
  }
}

template<typename T>
inline T queue<T>::pop() {
  RX_ASSERT(!is_empty(), "empty queue");

  auto this_node{m_first};

  T value{utility::move(this_node->m_value)};
  m_first = this_node->m_next;

  m_allocator->destroy<T>(this_node);

  return value;
}

template<typename T>
inline void queue<T>::clear() {
  while (!is_empty()) {
    pop();
  }
}

template<typename T>
inline memory::allocator* queue<T>::allocator() const {
  return m_allocator;
}

} // namespace rx

#endif // RX_CORE_QUEUE_H
