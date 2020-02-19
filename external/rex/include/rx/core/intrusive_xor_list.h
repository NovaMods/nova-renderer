#ifndef RX_CORE_INTRUSIVE_XOR_LIST_H
#define RX_CORE_INTRUSIVE_XOR_LIST_H
#include "rx/core/types.h"
#include "rx/core/utility/move.h"
#include "rx/core/concepts/no_copy.h"
#include "rx/core/hints/empty_bases.h"

namespace rx {

// # XOR doubly-linked-list
//
// An intrusive, doubly-linked list where nodes require half the storage since
// the link structure only requires one pointer instead of two.
//
// Like all intrusive containers, you must embed the node in your own structure
// which is called a "link". The use of |push| takes a pointer to this node.
// Retrieving the data associated with the node requires finding the offset
// relative to the node to reconstruct, this link address is given to
// |enumerate_head|, |enumerate_tail| or |node::data| by pointer to the link
// node in your structure.
//
// This should not be used unless you need to reduce the storage costs of your
// nodes. Rex makes use of this for it's globals system since rx::global<T> is
// quite large and minimizing the size is worthwhile.
//
// 32-bit: 8 bytes
// 64-bit: 16 bytes
struct RX_HINT_EMPTY_BASES intrusive_xor_list
  : concepts::no_copy
{
  struct node;

  constexpr intrusive_xor_list();
  intrusive_xor_list(intrusive_xor_list&& xor_list_);
  intrusive_xor_list& operator=(intrusive_xor_list&& xor_list_);

  void push(node* _node);

  // 32-bit: 4 bytes
  // 64-bit: 8 bytes
  struct RX_HINT_EMPTY_BASES node
    : concepts::no_copy
  {
    constexpr node();
    node(node&& node_);
    node& operator=(node&& node_);

    template<typename T>
    const T* data(node T::*_link) const;

    template<typename T>
    T* data(node T::*_link);

  private:
    friend struct intrusive_xor_list;

    node* m_link;
  };

private:
  node* m_head;
  node* m_tail;

  // 32-bit: 12 bytes
  // 64-bit: 24 bytes
  struct RX_HINT_EMPTY_BASES iterator
    : concepts::no_copy
  {
    constexpr iterator(node* _node);
    iterator(iterator&& iterator_);
    iterator& operator=(iterator&& iterator_);

    void next();
    void prev();

  protected:
    node* m_this;
    node* m_prev;
    node* m_next;
  };

public:
  // 32-bit: 12 (base class) + 4 bytes
  // 64-bit: 24 (base class) + 8 bytes
  template<typename T>
  struct enumerate
    : iterator
  {
    enumerate(enumerate&& enumerate_);
    enumerate& operator=(enumerate&& enumerate_);

    operator bool() const;

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    T* data();
    const T* data() const;

  private:
    friend struct intrusive_xor_list;
    constexpr enumerate(node* _root, node T::*_link);

    node T::*m_link;
  };

  template<typename T>
  enumerate<T> enumerate_head(node T::*_link) const;
  template<typename T>
  enumerate<T> enumerate_tail(node T::*_link) const;

  bool is_empty() const;
};

// intrusive_xor_list
inline constexpr intrusive_xor_list::intrusive_xor_list()
  : m_head{nullptr}
  , m_tail{nullptr}
{
}

inline intrusive_xor_list::intrusive_xor_list(intrusive_xor_list&& xor_list_)
  : m_head{xor_list_.m_head}
  , m_tail{xor_list_.m_tail}
{
  xor_list_.m_head = nullptr;
  xor_list_.m_tail = nullptr;
}

inline intrusive_xor_list& intrusive_xor_list::operator=(intrusive_xor_list&& xor_list_) {
  m_head = xor_list_.m_head;
  m_tail = xor_list_.m_tail;
  xor_list_.m_head = nullptr;
  xor_list_.m_tail = nullptr;
  return *this;
}

template<typename T>
inline intrusive_xor_list::enumerate<T> intrusive_xor_list::enumerate_head(node T::*_link) const {
  return {m_head, _link};
}

template<typename T>
inline intrusive_xor_list::enumerate<T> intrusive_xor_list::enumerate_tail(node T::*_link) const {
  return {m_tail, _link};
}

inline bool intrusive_xor_list::is_empty() const {
  return m_head == nullptr;
}

// intrusive_xor_list::node
inline constexpr intrusive_xor_list::node::node()
  : m_link{nullptr}
{
}

template<typename T>
inline const T* intrusive_xor_list::node::data(node T::*_link) const {
  const auto this_address = reinterpret_cast<rx_uintptr>(this);
  const auto link_offset = &(reinterpret_cast<const volatile T*>(0)->*_link);
  const auto link_address = reinterpret_cast<rx_uintptr>(link_offset);
  return reinterpret_cast<const T*>(this_address - link_address);
}

template<typename T>
inline T* intrusive_xor_list::node::data(node T::*_link) {
  const auto this_address = reinterpret_cast<rx_uintptr>(this);
  const auto link_offset = &(reinterpret_cast<const volatile T*>(0)->*_link);
  const auto link_address = reinterpret_cast<rx_uintptr>(link_offset);
  return reinterpret_cast<T*>(this_address - link_address);
}

// intrusive_xor_list::iterator
inline constexpr intrusive_xor_list::iterator::iterator(node* _node)
  : m_this{_node}
  , m_prev{nullptr}
  , m_next{nullptr}
{
}

inline intrusive_xor_list::iterator::iterator(iterator&& iterator_)
  : m_this{iterator_.m_this}
  , m_prev{iterator_.m_prev}
  , m_next{iterator_.m_next}
{
  iterator_.m_this = nullptr;
  iterator_.m_prev = nullptr;
  iterator_.m_next = nullptr;
}

inline intrusive_xor_list::iterator& intrusive_xor_list::iterator::operator=(iterator&& iterator_) {
  m_this = iterator_.m_this;
  m_prev = iterator_.m_prev;
  m_next = iterator_.m_next;

  iterator_.m_this = nullptr;
  iterator_.m_prev = nullptr;
  iterator_.m_next = nullptr;

  return *this;
}

// intrusive_xor_list::enumerate
template<typename T>
inline intrusive_xor_list::enumerate<T>::enumerate(enumerate&& enumerate_)
  : iterator{utility::move(enumerate_)}
  , m_link{enumerate_.m_link}
{
  enumerate_.m_link = nullptr;
}

template<typename T>
inline intrusive_xor_list::enumerate<T>& intrusive_xor_list::enumerate<T>::operator=(enumerate&& enumerate_) {
  iterator::operator=(utility::move(enumerate_));
  m_link = enumerate_.m_link;
  enumerate_.m_link = nullptr;
  return *this;
}

template<typename T>
inline constexpr intrusive_xor_list::enumerate<T>::enumerate(node* _root, node T::*_link)
  : iterator{_root}
  , m_link{_link}
{
}

template<typename T>
inline intrusive_xor_list::enumerate<T>::operator bool() const {
  return m_this != nullptr;
}

template<typename T>
inline T& intrusive_xor_list::enumerate<T>::operator*() {
  return *data();
}

template<typename T>
inline const T& intrusive_xor_list::enumerate<T>::operator*() const {
  return *data();
}

template<typename T>
inline T* intrusive_xor_list::enumerate<T>::operator->() {
  return data();
}

template<typename T>
inline const T* intrusive_xor_list::enumerate<T>::operator->() const {
  return data();
}

template<typename T>
inline T* intrusive_xor_list::enumerate<T>::data() {
  return m_this->data<T>(m_link);
}

template<typename T>
inline const T* intrusive_xor_list::enumerate<T>::data() const {
  return m_this->data<T>(m_link);
}

} // namespace rx

#endif // RX_CORE_XOR_LIST_H
