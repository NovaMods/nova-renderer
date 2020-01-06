#ifndef RX_CORE_CONCURRENCY_FUTURE_H
#define RX_CORE_CONCURRENCY_FUTURE_H
#include "rx/core/concurrency/atomic.h"
#include "rx/core/concurrency/mutex.h"
#include "rx/core/concurrency/condition_variable.h"

#include "rx/core/memory/uninitialized_storage.h"
#include "rx/core/memory/system_allocator.h"

namespace rx::concurrency {

namespace detail {

template<typename T>
struct shared {
  shared(memory::allocator* _allocator)
    : m_allocator{_allocator}
    , m_count{1}
    , m_ready{false}
  {
  }

  ~shared() {
    m_storage.fini();
  }

  void release() {
    if (--m_count == 0) {
      m_allocator->destroy<shared<T>>(this);
    }
  }

  shared* acquire() {
    ++m_count;
    return this;
  }

  void set(const T& _value) {
    scope_lock lock(m_mutex);
    RX_ASSERT(!m_ready, "already ready");
    m_storage.init(_value);
    make_ready();
  }

  void set(T&& value_) {
    scope_lock lock(m_mutex);
    RX_ASSERT(!m_ready, "already ready");
    m_storage.init(utility::move(value_));
    make_ready();
  }

  void wait() const {
    if (!is_ready()) {
      scope_lock lock(m_mutex);
      m_condition_variable.wait(lock, [this] { return m_ready.load(); });
    }
  }

  T& get() {
    wait();
    return *m_storage.data();
  }

  void make_ready() {
    m_ready.store(true);
    m_condition_variable.signal();
  }

  bool is_ready() const {
    return m_ready.load();
  }

  memory::allocator* m_allocator;
  memory::uninitialized_storage<T> m_storage;
  mutable mutex m_mutex;
  mutable condition_variable m_condition_variable;
  atomic<rx_size> m_count;
  atomic<bool> m_ready;
};

} // namespace detail

template<typename T>
struct future {
  future()
    : m_state{nullptr}
  {
  }

  future(future&& future_)
    : m_state{future_.m_state}
  {
    future_.m_state = nullptr;
  }

  future(const future& _future)
    : m_state{_future.acquire()}
  {
  }

  future& operator=(future&& future_) {
    RX_ASSERT(&future_ != this, "self assignment");

    release();
    m_state = future_.m_state;
    future_.m_state = nullptr;
    return *this;
  }

  future& operator=(const future& _future) {
    RX_ASSERT(&_future != this, "self assignment");

    release();
    m_state = _future.acquire();
    return *this;
  }

  ~future() {
    release();
  }

  void wait() const {
    RX_ASSERT(m_state, "no shared state");
    m_state->wait();
  }

  bool is_valid() const {
    return m_state != nullptr;
  }

  bool is_ready() const {
    RX_ASSERT(m_state, "no shared state");
    return m_state->is_ready();
  }

  T& get() {
    RX_ASSERT(m_state, "no shared state");
    return m_state->get();
  }

private:
  template<typename>
  friend struct promise;

  future(detail::shared<T>* _state)
    : m_state{_state->acquire()}
  {
  }

  void release() {
    if (m_state) {
      m_state->release();
    }
  }

  detail::shared<T>* acquire() const {
    return m_state ? m_state->acquire() : nullptr;
  }

  detail::shared<T>* m_state;
};

template<typename T>
struct promise {
  promise()
    : promise{&memory::g_system_allocator}
  {
  }

  promise(memory::allocator* _allocator)
    : m_state{_allocator->create<detail::shared<T>>(_allocator)}
  {
  }

  promise(promise&& promise_)
    : m_state{promise_.m_state}
  {
    promise_.m_state = nullptr;
  }

  promise(const promise& _promise)
    : m_state{_promise.acquire()}
  {
  }

  promise& operator=(promise&& promise_) {
    RX_ASSERT(&promise_ != this, "self assignment");

    release();
    m_state = promise_.m_state;
    promise_.m_state = nullptr;
    return *this;
  }

  promise& operator=(const promise& _promise) {
    RX_ASSERT(&_promise != this, "self assignment");

    release();
    m_state = _promise.acquire();
    return *this;
  }

  ~promise() {
    release();
  }

  future<T> make_future() {
    return {m_state};
  }

  void set(const T& _value) {
    RX_ASSERT(m_state, "no shared state");
    m_state->set(_value);
  }

  void set(T&& value_) {
    RX_ASSERT(m_state, "no shared state");
    m_state->set(utility::move(value_));
  }

private:
  void release() {
    if (m_state) {
      m_state->release();
    }
  }

  detail::shared<T>* acquire() const {
    return m_state ? m_state->acquire() : nullptr;
  }

  detail::shared<T>* m_state;
};

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_FUTURE_H
