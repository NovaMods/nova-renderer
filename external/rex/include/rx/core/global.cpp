#include <string.h> // strcmp

#include "rx/core/global.h"
#include "rx/core/log.h"

#include "rx/core/concurrency/scope_lock.h"
#include "rx/core/concurrency/spin_lock.h"

namespace rx {

RX_LOG("global", logger);

static concurrency::spin_lock g_lock;

// global_node
void global_node::init_global() {
  if (m_enabled) {
    RX_ASSERT(!m_initialized, "already initialized");
    logger(log::level::k_verbose, "%p init: %s/%s", this, m_group, m_name);
    m_init_global(m_global_store, m_argument_store);
    m_initialized = true;
  }
}

void global_node::fini_global() {
  if (m_enabled) {
    RX_ASSERT(m_initialized, "not initialized");
    logger(log::level::k_verbose, "%p fini: %s/%s", this, m_group, m_name);
    m_fini_global(m_global_store);
    if (m_fini_arguments) {
      m_fini_arguments(m_argument_store);
    }
    m_initialized = false;
  }
}

void global_node::init() {
  RX_ASSERT(!m_initialized, "already initialized");
  m_init_global(m_global_store, m_argument_store);
  m_enabled = false;
  m_initialized = true;
}

void global_node::fini() {
  RX_ASSERT(m_initialized, "not initialized");
  m_fini_global(m_global_store);
  if (m_fini_arguments) {
    m_fini_arguments(m_argument_store);
  }
  m_enabled = false;
  m_initialized = false;
}

// global_group
global_node* global_group::find(const char* _name) {
  for (global_node* node{m_head}; node; node = node->m_next_grouped) {
    if (!strcmp(node->m_name, _name)) {
      return node;
    }
  }
  return nullptr;
}

void global_group::init() {
  for (global_node* node{m_head}; node; node = node->m_next_grouped) {
    node->init();
  }
}

void global_group::fini() {
  for (global_node* node{m_tail}; node; node = node->m_prev_grouped) {
    node->fini();
  }
}

void global_group::init_global() {
  for (global_node* node{m_head}; node; node = node->m_next_grouped) {
    node->init_global();
  }
}

void global_group::fini_global() {
  for (global_node* node{m_tail}; node; node = node->m_prev_grouped) {
    node->fini_global();
  }
}

// globals
global_group* globals::find(const char* _name) {
  for (global_group* group{s_group_head}; group; group = group->m_next) {
    if (!strcmp(group->m_name, _name)) {
      return group;
    }
  }
  return nullptr;
}

void globals::link() {
  concurrency::scope_lock lock{g_lock};
  for (global_group* group{s_group_head}; group; group = group->m_next) {
    for (global_node* node{s_node_head}; node; node = node->m_next_ungrouped) {
      if (!strcmp(node->m_group, group->m_name)) {
        if (!group->m_head) {
          group->m_head = node;
        }
        if (group->m_tail) {
          node->m_prev_grouped = group->m_tail;
          node->m_prev_grouped->m_next_grouped = node;
        }
        group->m_tail = node;
      }
    }
  }
}

void globals::init() {
  for (global_group* group{s_group_head}; group; group = group->m_next) {
    group->init_global();
  }
}

void globals::fini() {
  for (global_group* group{s_group_tail}; group; group = group->m_prev) {
    group->fini_global();
  }
}

void globals::link(global_node* _node) {
  concurrency::scope_lock lock{g_lock};
  if (!s_node_head) {
    s_node_head = _node;
  }
  if (s_node_tail) {
    _node->m_prev_ungrouped = s_node_tail;
    _node->m_prev_ungrouped->m_next_ungrouped = _node;
  }
  s_node_tail = _node;
}

void globals::link(global_group* _group) {
  concurrency::scope_lock lock{g_lock};
  if (!s_group_head) {
    s_group_head = _group;
  }
  if (s_group_tail) {
    _group->m_prev = s_group_tail;
    _group->m_prev->m_next = _group;
  }
  s_group_tail = _group;
}

static RX_GLOBAL_GROUP("system", g_group_system);

} // namespace rx
