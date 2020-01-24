#ifndef RX_CORE_CONCURRENCY_SCOPE_UNLOCK_H
#define RX_CORE_CONCURRENCY_SCOPE_UNLOCK_H

namespace rx::concurrency {

// generic scoped unlock
template<typename T>
struct scope_unlock {
  explicit constexpr scope_unlock(T& lock_);
  ~scope_unlock();
private:
  T& m_lock;
};

template<typename T>
inline constexpr scope_unlock<T>::scope_unlock(T& lock_)
  : m_lock{lock_}
{
  m_lock.unlock();
}

template<typename T>
inline scope_unlock<T>::~scope_unlock() {
  m_lock.lock();
}

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_SCOPE_UNLOCK_H
