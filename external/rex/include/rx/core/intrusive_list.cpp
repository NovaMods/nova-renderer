#include "rx/core/intrusive_list.h"

namespace rx {

void intrusive_list::push_front(node* node_) {
  if (m_head) {
    m_head->m_prev = node_;
    node_->m_prev = nullptr;
    node_->m_next = m_head;
    m_head = node_;
  } else {
    m_head = node_;
    m_tail = node_;
    node_->m_next = nullptr;
    node_->m_prev = nullptr;
  }
}

void intrusive_list::push_back(node* node_) {
  if (m_tail) {
    m_tail->m_next = node_;
    node_->m_prev = m_tail;
    node_->m_next = nullptr;
    m_tail = node_;
  } else {
    m_head = node_;
    m_tail = node_;
    node_->m_next = nullptr;
    node_->m_prev = nullptr;
  }
}

intrusive_list::node* intrusive_list::pop_front() {
  if (node* link = m_head) {
    if (link->m_next) {
      link->m_next->m_prev = link->m_prev;
    }
    if (link->m_prev) {
      link->m_prev->m_next = link->m_next;
    }
    if (link == m_head) {
      m_head = link->m_next;
    }
    if (link == m_tail) {
      m_tail = link->m_prev;
    }
    return link;
  }
  return nullptr;
}

intrusive_list::node* intrusive_list::pop_back() {
  if (node* link = m_tail) {
    if (link->m_next) {
      link->m_next->m_prev = link->m_prev;
    }
    if (link->m_prev) {
      link->m_prev->m_next = link->m_next;
    }
    if (link == m_head) {
      m_head = link->m_next;
    }
    if (link == m_tail) {
      m_tail = link->m_prev;
    }
    return link;
  }
  return nullptr;
}

} // namespace rx
