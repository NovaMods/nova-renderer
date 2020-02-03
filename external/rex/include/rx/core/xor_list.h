#ifndef RX_CORE_XOR_LIST_H
#define RX_CORE_XOR_LIST_H
#include "rx/core/types.h"

namespace rx {

// # XOR doubly-linked-list
//
// An intrusive, doubly-linked list where nodes require half the storage since
// the link structure only requires one pointer.
//
// Like all intrusive containers, you must embed the node in your own structure
// which is called a "link". The use of |push| takes a pointer to this node.
// Retrieving the data associated with the node requires finding the offset
// relative to the node to reconstruct, this link address is given to the
// enumerate structure or |node::data|.
//
// This should not be used unless you need to reduce the storage costs of your
// nodes. Rex makes use of this for it's globals system since rx::global<T> is
// quite large.
//
// 32-bit: 8 bytes
// 64-bit: 16 bytes
struct xor_list {
  // 32-bit: 4 bytes
  // 64-bit: 8 bytes
  struct node {
    constexpr node();

    template<typename T>
    const T* data(rx_size _offset) const;

    template<typename T>
    T* data(rx_size _offset);

  private:
    friend struct xor_list;

    node* m_link;
  };

  constexpr xor_list();

  void push(node* _node);

  node* head() const;
  node* tail() const;

private:
  node* m_head;
  node* m_tail;

  // 32-bit: 12 bytes
  // 64-bit: 24 bytes
  struct iterator {
    constexpr iterator(node* _node);

    void next();

  protected:
    node* m_this;
    node* m_prev;
    node* m_next;
  };

public:
  // 32-bit: 12 (base class) + 4 bytes
  // 64-bit: 24 (base class) + 8 bytes
  template<typename T>
  struct enumerate : iterator {
    enumerate(node* _root, node T::*link);

    operator bool() const;

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    T* data();
    const T* data() const;

  private:
    rx_size m_link_offset;
  };
};

inline constexpr xor_list::node::node()
  : m_link{nullptr}
{
}

template<typename T>
inline const T* xor_list::node::data(rx_size _offset) const {
  const auto address = reinterpret_cast<rx_uintptr>(this);
  return reinterpret_cast<const T*>(address - _offset);
}

template<typename T>
inline T* xor_list::node::data(rx_size _offset) {
  const auto address = reinterpret_cast<rx_uintptr>(this);
  return reinterpret_cast<T*>(address - _offset);
}

inline constexpr xor_list::xor_list()
  : m_head{nullptr}
  , m_tail{nullptr}
{
}

inline xor_list::node* xor_list::head() const {
  return m_head;
}

inline xor_list::node* xor_list::tail() const {
  return m_tail;
}

inline constexpr xor_list::iterator::iterator(node* _node)
  : m_this{_node}
  , m_prev{nullptr}
  , m_next{nullptr}
{
}

template<typename T>
inline xor_list::enumerate<T>::enumerate(node* _root, node T::*link)
  : iterator{_root}
{
  // NOTE(dweiler): This invokes undefined behhavior according to the C++
  // standard. It's technically possible to store the `node T::*link` and do
  // a subtraction from |m_this| to get the relative address which would avoid
  // undefined behavior, however I don't see a simple way of doing that
  // currently.
  //
  // For now, just synthesize a null-pointer, dereference it with pointer to
  // member representing the link, then take the address of it, which will be
  // relative to null-pointer, which is 0. This is the "offset" in the |T|
  // structure where the link can be found.
  const auto relative_address
    = &(reinterpret_cast<const volatile T*>(0)->*link);

  m_link_offset = reinterpret_cast<rx_size>(relative_address);
}

template<typename T>
inline xor_list::enumerate<T>::operator bool() const {
  return m_this != nullptr;
}

template<typename T>
inline T& xor_list::enumerate<T>::operator*() {
  return *data();
}

template<typename T>
inline const T& xor_list::enumerate<T>::operator*() const {
  return *data();
}

template<typename T>
inline T* xor_list::enumerate<T>::operator->() {
  return data();
}

template<typename T>
inline const T* xor_list::enumerate<T>::operator->() const {
  return data();
}

template<typename T>
inline T* xor_list::enumerate<T>::data() {
  return m_this->data<T>(m_link_offset);
}

template<typename T>
inline const T* xor_list::enumerate<T>::data() const {
  return m_this->data<T>(m_link_offset);
}

} // namespace rx

#endif // RX_CORE_XOR_LIST_H
