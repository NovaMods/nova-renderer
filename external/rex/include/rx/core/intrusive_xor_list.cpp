#include "rx/core/intrusive_xor_list.h"
#include "rx/core/types.h"

namespace rx {

static intrusive_xor_list::node* xor_nodes(intrusive_xor_list::node* _x, intrusive_xor_list::node* _y) {
  const auto x = reinterpret_cast<rx_uintptr>(_x);
  const auto y = reinterpret_cast<rx_uintptr>(_y);
  return reinterpret_cast<intrusive_xor_list::node*>(x ^ y);
}

void intrusive_xor_list::push(node* node_) {
  if (!m_head) {
    m_head = node_;
    m_tail = node_;
  } else {
    node_->m_link = xor_nodes(m_tail, nullptr);
    m_tail->m_link = xor_nodes(node_, xor_nodes(m_tail->m_link, nullptr));
    m_tail = node_;
  }
}

void intrusive_xor_list::iterator::next() {
  if (m_this) {
    m_next = xor_nodes(m_prev, m_this->m_link);
    m_prev = m_this;
    m_this = m_next;
  }
}

void intrusive_xor_list::iterator::prev() {
  next();
}

} // namespace rx
