#ifndef RX_CORE_GLOBAL_H
#define RX_CORE_GLOBAL_H
#include "rx/core/memory/uninitialized_storage.h"
#include "rx/core/assert.h"

namespace rx {

struct global_group;

struct global_node {
  template<typename T, rx_size E, typename... Ts>
  global_node(const char* _group, const char* _name,
    memory::uninitialized_storage<T>& _storage, rx_byte (&_argument_store)[E],
    Ts&&... _arguments);

  void init();
  void fini();

  template<typename... Ts>
  void init(Ts&&... _arguments);

  const char* name() const;

  rx_byte* data();
  const rx_byte* data() const;

  template<typename T>
  T* cast();

  template<typename T>
  const T* cast() const;

private:
  friend struct globals;
  friend struct global_group;

  void init_global();
  void fini_global();

  template<typename F, typename... Rs>
  struct arguments : arguments<Rs...> {
    constexpr arguments(F&& _first, Rs&&... _rest)
      : arguments<Rs...>(utility::forward<Rs>(_rest)...)
      , first{utility::forward<F>(_first)}
    {
    }
    F first;
  };
  template<typename F>
  struct arguments<F> {
    constexpr arguments(F&& _first)
      : first{utility::forward<F>(_first)}
    {
    }
    F first;
  };

  template<rx_size I, typename F, typename... Rs>
  struct read_argument {
    static auto value(const arguments<F, Rs...>* _arguments) {
      return read_argument<I - 1, Rs...>::value(_arguments);
    }
  };
  template<typename F, typename... Rs>
  struct read_argument<0, F, Rs...> {
    static F value(const arguments<F, Rs...>* _arguments) {
      return _arguments->first;
    }
  };
  template<rx_size I, typename F, typename... Rs>
  static auto argument(const arguments<F, Rs...>& _arguments) {
    return read_argument<I, F, Rs...>::value(&_arguments);
  }

  template<rx_size...>
  struct unpack_sequence {};
  template<rx_size N, rx_size... Ns>
  struct unpack_arguments
    : unpack_arguments<N - 1, N - 1, Ns...>
  {
  };
  template<rx_size... Ns>
  struct unpack_arguments<0, Ns...> {
    using type = unpack_sequence<Ns...>;
  };

  template<typename... Ts>
  static void construct_arguments(rx_byte* _argument_store, Ts&&... _arguments) {
    utility::construct<arguments<Ts...>>(_argument_store,
      utility::forward<Ts>(_arguments)...);
  }

  template<typename... Ts>
  static void destruct_arguments(rx_byte* _argument_store) {
    utility::destruct<arguments<Ts...>>(_argument_store);
  }

  template<typename T, typename... Ts>
  static void construct_global(rx_byte* _storage, rx_byte* _argument_store) {
    construct_global<T, Ts...>(typename unpack_arguments<sizeof...(Ts)>::type{},
      _storage, _argument_store);
  }

  template<typename T, typename... Ts, rx_size... Ns>
  static void construct_global(unpack_sequence<Ns...>, rx_byte* _storage,
    [[maybe_unused]] rx_byte* _argument_store)
  {
    utility::construct<T>(_storage,
      argument<Ns>(*reinterpret_cast<arguments<Ts...>*>(_argument_store))...);
  }

  template<typename T>
  static void destruct_global(rx_byte* _data) {
    utility::destruct<T>(_data);
  }

  rx_byte* m_global_store;
  rx_byte* m_argument_store;

  struct {
    rx_u16 size;
    rx_u16 alignment;
  } m_global_traits;

  void (*m_init_global)(rx_byte* _storage, rx_byte* _argument_store);
  void (*m_fini_global)(rx_byte* _storage);
  void (*m_fini_arguments)(rx_byte* _argument_store);

  // Links for group-local linked-list of nodes.
  global_node* m_next_grouped;
  global_node* m_prev_grouped;

  // Links for global linked-list of nodes.
  global_node* m_next_ungrouped;
  global_node* m_prev_ungrouped;

  const char* m_group;
  const char* m_name;

  bool m_enabled;
  bool m_initialized;
};

template<typename T>
struct global {
  template<typename... Ts>
  global(const char* _group, const char* _name, Ts&&... _arguments);

  constexpr const char* name() const;

  constexpr T* operator&();
  constexpr const T* operator&() const;

  constexpr T& operator*();
  constexpr const T& operator*() const;

  constexpr T* operator->();
  constexpr const T* operator->() const;

  template<typename... Ts>
  auto operator()(Ts&&... _arguments);

  constexpr T* data();
  constexpr const T* data() const;

private:
  memory::uninitialized_storage<T> m_global_store;
  rx_byte m_argument_store[64];
  global_node m_node;
};

struct global_group {
  constexpr global_group(const char* _name);

  constexpr const char* name() const;

  global_node* find(const char* _name);

  void init();
  void fini();

  template<typename F>
  void each(F&& _function);

private:
  friend struct globals;
  friend struct global_node;

  void init_global();
  void fini_global();

  const char* m_name;

  // Linked-list of nodes for this group.
  global_node* m_head;
  global_node* m_tail;

  // Links for global linked-list of groups in |globals|
  global_group* m_next;
  global_group* m_prev;
};

struct globals {
  static global_group* find(const char* _name);

  // Goes over global linked-list of groups, adding nodes to the group
  // if the |global_node::m_group| matches the group name.
  static void link();

  static void init();
  static void fini();

private:
  friend struct global_node;
  friend struct global_group;

  static void link(global_node* _node);
  static void link(global_group* _group);

  // Global linked-list of "groups"
  static inline global_group* s_group_head;
  static inline global_group* s_group_tail;

  // Global linked-list of "nodes"
  static inline global_node* s_node_head;
  static inline global_node* s_node_tail;
};

// global_node
template<typename T, rx_size E, typename... Ts>
inline global_node::global_node(const char* _group, const char* _name,
  memory::uninitialized_storage<T>& _storage, rx_byte (&_argument_store)[E],
  Ts&&... _arguments)
  : m_global_store{reinterpret_cast<rx_byte*>(_storage.data())}
  , m_argument_store{_argument_store}
  , m_init_global{construct_global<T, Ts...>}
  , m_fini_global{destruct_global<T>}
  , m_fini_arguments{nullptr}
  , m_next_grouped{nullptr}
  , m_prev_grouped{nullptr}
  , m_next_ungrouped{nullptr}
  , m_prev_ungrouped{nullptr}
  , m_group{_group ? _group : "system"}
  , m_name{_name}
  , m_enabled{true}
  , m_initialized{false}
{
  m_global_traits.size = sizeof(T);
  m_global_traits.alignment = alignof(T);

  if constexpr (sizeof...(Ts) != 0) {
    static_assert(sizeof(arguments<Ts...>) <= sizeof _argument_store,
      "too much constructor data to store for global");

    construct_arguments<Ts...>(m_argument_store, utility::forward<Ts>(_arguments)...);
    m_fini_arguments = destruct_arguments<Ts...>;
  }

  globals::link(this);
}

template<typename... Ts>
inline void global_node::init(Ts&&... _arguments) {
  static_assert(sizeof...(Ts) != 0,
    "use void init() for default construction");

  if (m_fini_arguments) {
    m_fini_arguments(m_argument_store);
  }

  construct_arguments<Ts...>(m_argument_store, utility::forward<Ts>(_arguments)...);

  init();
}

inline const char* global_node::name() const {
  return m_name;
}

inline rx_byte* global_node::data() {
  return m_global_store;
}

inline const rx_byte* global_node::data() const {
  return m_global_store;
}

template<typename T>
inline T* global_node::cast() {
  RX_ASSERT(m_global_traits.size == sizeof(T)
    && m_global_traits.alignment == alignof(T), "invalid type cast");

  return reinterpret_cast<T*>(m_global_store);
}

template<typename T>
inline const T* global_node::cast() const {
  RX_ASSERT(m_global_traits.size == sizeof(T)
    && m_global_traits.alignment == alignof(T), "invalid type cast");
  return reinterpret_cast<const T*>(m_global_store);
}

// global_group
inline constexpr global_group::global_group(const char* _name)
  : m_name{_name}
  , m_head{nullptr}
  , m_tail{nullptr}
  , m_next{nullptr}
  , m_prev{nullptr}
{
  globals::link(this);
}

inline constexpr const char* global_group::name() const {
  return m_name;
}

template<typename F>
inline void global_group::each(F&& _function) {
  for (global_node* node{m_head}; node; node = node->m_next_grouped) {
    _function(node);
  }
}

// global
template<typename T>
template<typename... Ts>
inline global<T>::global(const char* _group, const char* _name, Ts&&... _arguments)
  : m_node{_group, _name, m_global_store, m_argument_store, utility::forward<Ts>(_arguments)...}
{
}

template<typename T>
inline constexpr const char* global<T>::name() const {
  return m_node.name();
}

template<typename T>
inline constexpr T* global<T>::operator&() {
  return m_global_store.data();
}

template<typename T>
inline constexpr const T* global<T>::operator&() const {
  return m_global_store.data();
}

template<typename T>
inline constexpr T& global<T>::operator*() {
  return *m_global_store.data();
}

template<typename T>
constexpr const T& global<T>::operator*() const {
  return *m_global_store.data();
}

template<typename T>
constexpr T* global<T>::operator->() {
  return m_global_store.data();
}

template<typename T>
constexpr const T* global<T>::operator->() const {
  return m_global_store.data();
}

template<typename T>
template<typename... Ts>
inline auto global<T>::operator()(Ts&&... _arguments) {
  return operator*()(utility::forward<Ts>(_arguments)...);
}

template<typename T>
inline constexpr T* global<T>::data() {
  return m_global_store.data();
}

template<typename T>
inline constexpr const T* global<T>::data() const {
  return m_global_store.data();
}

#define RX_GLOBAL \
  ::rx::global

#define RX_GLOBAL_GROUP(_name, _ident) \
  ::rx::global_group _ident {(_name)}

} // namespace rx

#endif // RX_CORE_GLOBAL_H
