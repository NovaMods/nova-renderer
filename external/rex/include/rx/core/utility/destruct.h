#ifndef RX_CORE_UTILITY_DESTRUCT_H
#define RX_CORE_UTILITY_DESTRUCT_H

namespace rx::utility {

template<typename T>
inline void destruct(void* _data) {
  reinterpret_cast<T*>(_data)->~T();
}

} // namespace rx::utility

#endif // RX_CORE_UTILITY_DESTRUCT_H
