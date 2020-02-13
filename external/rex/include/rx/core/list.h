#ifndef RX_CORE_LIST_H
#define RX_CORE_LIST_H
#include "rx/core/types.h"
#include "rx/core/concepts/no_copy.h"

namespace rx {

// # Doubly-linked-list
//
// An intrusive, doubly-linked list.
//
// Like all intrusive containers, you must embed the node in your own structure
// which is called a "link". The use of |push_front| and |push_back| takes a
// pointer to this node. Retrieving the data associated with the node requires
// finding the offset relative to the node to reconstruct, this link address is
// given to |enumerate_head|, |enumerate_tail| or |node::data| by pointer to
// the link node in your structure.
//
// 32-bit: 8 bytes
// 64-bit: 16 bytes
struct list
  : concepts::no_copy
{
  struct node;

  constexpr list();
  list(list&& _list);

  list& operator=(list&& list_);

  void push_front(node* _node);
  void push_back(node* _node);

  node* pop_front();
  node* pop_back();

  // 32-bit: 8 bytes
  // 64-bit: 16 bytes
  struct node
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
    friend struct list;

    node* m_next;
    node* m_prev;
  };

  // 32-bit: 8 bytes
  // 64-bit: 16 bytes
  template<typename T>
  struct enumerate
    : concepts::no_copy
  {
    constexpr enumerate(node* _root, node T::*_link);
    enumerate(enumerate&& enumerate_);
    enumerate& operator=(enumerate&& enumerate_);

    operator bool() const;

    T& operator*();
    const T& operator*() const;

    T* operator->();
    const T* operator->() const;

    T* data();
    const T* data() const;

    void next();
    void prev();

  private:
    node* m_this;
    node T::*m_link;
  };

  template<typename T>
  enumerate<T> enumerate_head(node T::*_link) const;
  template<typename T>
  enumerate<T> enumerate_tail(node T::*_link) const;

  bool is_empty() const;

private:
  node* m_head;
  node* m_tail;
};

// list
inline constexpr list::list()
  : m_head{nullptr}
  , m_tail{nullptr}
{
}

inline list::list(list&& list_)
  : m_head{list_.m_head}
  , m_tail{list_.m_tail}
{
  list_.m_head = nullptr;
  list_.m_tail = nullptr;
}

inline list& list::operator=(list&& list_) {
  m_head = list_.m_head;
  m_tail = list_.m_tail;
  list_.m_head = nullptr;
  list_.m_tail = nullptr;
  return *this;
}

template<typename T>
inline list::enumerate<T> list::enumerate_head(node T::*_link) const {
  return {m_head, _link};
}

template<typename T>
inline list::enumerate<T> list::enumerate_tail(node T::*_link) const {
  return {m_tail, _link};
}

inline bool list::is_empty() const {
  return m_head == nullptr;
}

// list::node
inline constexpr list::node::node()
  : m_next{nullptr}
  , m_prev{nullptr}
{
}

inline list::node::node(node&& node_)
  : m_next{node_.m_next}
  , m_prev{node_.m_prev}
{
  node_.m_next = nullptr;
  node_.m_prev = nullptr;
}

inline list::node& list::node::operator=(node&& node_) {
  m_next = node_.m_next;
  m_prev = node_.m_prev;
  node_.m_next = nullptr;
  node_.m_prev = nullptr;
  return *this;
}

template<typename T>
inline const T* list::node::data(node T::*_link) const {
  const auto this_address = reinterpret_cast<rx_uintptr>(this);
  const auto link_offset = &(reinterpret_cast<const volatile T*>(0)->*_link);
  const auto link_address = reinterpret_cast<rx_uintptr>(link_offset);
  return reinterpret_cast<const T*>(this_address - link_address);
}

template<typename T>
inline T* list::node::data(node T::*_link) {
  const auto this_address = reinterpret_cast<rx_uintptr>(this);
  const auto link_offset = &(reinterpret_cast<const volatile T*>(0)->*_link);
  const auto link_address = reinterpret_cast<rx_uintptr>(link_offset);
  return reinterpret_cast<T*>(this_address - link_address);
}

// list::enumerate
template<typename T>
inline constexpr list::enumerate<T>::enumerate(node* _root, node T::*_link)
  : m_this{_root}
  , m_link{_link}
{
}

template<typename T>
inline list::enumerate<T>::enumerate(enumerate&& enumerate_)
  : m_this{enumerate_.m_this}
  , m_link{enumerate_.m_link}
{
  enumerate_.m_this = nullptr;
  enumerate_.m_link = nullptr;
}

template<typename T>
inline list::enumerate<T>& list::enumerate<T>::operator=(enumerate&& enumerate_) {
  m_this = enumerate_.m_this;
  m_link = enumerate_.m_link;
  enumerate_.m_this = nullptr;
  enumerate_.m_link = nullptr;
  return *this;
}

template<typename T>
inline list::enumerate<T>::operator bool() const {
  return m_this;
}

template<typename T>
inline void list::enumerate<T>::next() {
  m_this = m_this->m_next;
}

template<typename T>
inline void list::enumerate<T>::prev() {
  m_this = m_this->m_prev;
}

template<typename T>
inline T& list::enumerate<T>::operator*() {
  return *data();
}

template<typename T>
inline const T& list::enumerate<T>::operator*() const {
  return *data();
}

template<typename T>
inline T* list::enumerate<T>::operator->() {
  return data();
}

template<typename T>
inline const T* list::enumerate<T>::operator->() const {
  return data();
}

template<typename T>
inline T* list::enumerate<T>::data() {
  return m_this->data<T>(m_link);
}

template<typename T>
inline const T* list::enumerate<T>::data() const {
  return m_this->data<T>(m_link);
}

} // namespace rx

#endif // RX_CORE_LIST_H
