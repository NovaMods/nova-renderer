#ifndef RX_CORE_GLOBAL_H
#define RX_CORE_GLOBAL_H
#include "rx/core/memory/uninitialized_storage.h"
#include "rx/core/assert.h"
#include "rx/core/intrusive_xor_list.h"

namespace rx {

// Constant, sharable properties for a given global<T>. Nodes get a pointer
// to static instance data of this, allowing them to avoid storing this data
// themselves.
//
// This saves 8 bytes in global_node for 32-bit.
// This saves 12 bytes in global_node for 64-bit.
struct global_shared {
  rx_u16 size;
  rx_u16 alignment;
  void (*finalizer)(void* _data);
};

struct global_node {
  template<typename T, typename... Ts>
  global_node(const char* _group, const char* _name,
    memory::uninitialized_storage<T>& _storage, const global_shared* _shared,
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
  static auto argument(const arguments<F, Rs...>* _arguments) {
    return read_argument<I, F, Rs...>::value(_arguments);
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
      argument<Ns>(reinterpret_cast<arguments<Ts...>*>(_argument_store))...);
  }

  const global_shared* m_shared;

  void (*m_init_global)(rx_byte* _storage, rx_byte* _argument_store);
  void (*m_fini_arguments)(rx_byte* _argument_store);

  intrusive_xor_list::node m_grouped;
  intrusive_xor_list::node m_ungrouped;

  const char* m_group;
  const char* m_name;

  enum : rx_u16 {
    k_enabled     = 1 << 0,
    k_initialized = 1 << 1
  };

  rx_u16 m_global_store;
  rx_u16 m_flags;

  // This needs to be aligned otherwise `arguments` will invoke undefined behavior.
  alignas(16) rx_byte m_argument_store[64];
};

template<typename T>
struct global {
  static inline const global_shared s_shared = {
    sizeof(T),
    alignof(T),
    &utility::destruct<T>
  };

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
  global_node m_node;
};

struct global_group {
  global_group(const char* _name);

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

  // Nodes for this group. This is constructed after a call to |globals::link|.
  intrusive_xor_list m_list;

  // Link for global linked-list of groups in |globals|.
  intrusive_xor_list::node m_link;
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

  // Global linked-list of groups.
  static inline intrusive_xor_list s_group_list;

  // Global linked-list of ungrouped nodes.
  static inline intrusive_xor_list s_node_list;
};

// global_node
template<typename T, typename... Ts>
inline global_node::global_node(const char* _group, const char* _name,
  memory::uninitialized_storage<T>& _global_store, const global_shared* _shared,
  Ts&&... _arguments)
  : m_shared{_shared}
  , m_init_global{construct_global<T, Ts...>}
  , m_fini_arguments{nullptr}
  , m_group{_group ? _group : "system"}
  , m_name{_name}
  , m_flags{k_enabled}
{
  // |global<T>| embeds uninitialized_storage<T> with |global_node| proceeding
  // it, by taking the difference of |this - _global_storage.data()| we can
  // determine how many bytes in memory from this object, backwards is the
  // storage for the node. We do this to save space, since we only need two
  // bytes to store this.
  //
  // This saves 2-bytes in global_node for 32-bit.
  // This saves 6-bytes in global_node for 64-bit.
  const rx_uintptr this_address = reinterpret_cast<rx_uintptr>(this);
  const rx_uintptr store_address = reinterpret_cast<rx_uintptr>(_global_store.data());
  const rx_uintptr difference = this_address - store_address;
  RX_ASSERT(difference <= 0xffff, "global is too large");
  m_global_store = static_cast<rx_u32>(difference);

  if constexpr (sizeof...(Ts) != 0) {
    static_assert(sizeof(arguments<Ts...>) <= sizeof m_argument_store,
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

  construct_arguments<traits::remove_reference<Ts>...>
    (m_argument_store, utility::forward<traits::remove_reference<Ts>>(_arguments)...);
  init();
}

inline const char* global_node::name() const {
  return m_name;
}

inline rx_byte* global_node::data() {
  // Reconstruct the storage pointer from |m_global_store|.
  return reinterpret_cast<rx_byte*>(reinterpret_cast<rx_uintptr>(this) - m_global_store);
}

inline const rx_byte* global_node::data() const {
  // Reconstruct the storage pointer from |m_global_store|.
  return reinterpret_cast<const rx_byte*>(reinterpret_cast<rx_uintptr>(this) - m_global_store);
}

template<typename T>
inline T* global_node::cast() {
  RX_ASSERT(m_shared->size == sizeof(T), "invalid type cast");
  RX_ASSERT(m_shared->alignment == alignof(T), "invalid type cast");
  return reinterpret_cast<T*>(data());
}

template<typename T>
inline const T* global_node::cast() const {
  RX_ASSERT(m_shared->size == sizeof(T), "invalid type cast");
  RX_ASSERT(m_shared->alignment == alignof(T), "invalid type cast");
  return reinterpret_cast<const T*>(data());
}

// global_group
inline global_group::global_group(const char* _name)
  : m_name{_name}
{
  globals::link(this);
}

inline constexpr const char* global_group::name() const {
  return m_name;
}

template<typename F>
inline void global_group::each(F&& _function) {
  for (auto node = m_list.enumerate_head(&global_node::m_grouped); node; node.next()) {
    _function(node.data());
  }
}

// global
template<typename T>
template<typename... Ts>
inline global<T>::global(const char* _group, const char* _name, Ts&&... _arguments)
  : m_node{_group, _name, m_global_store, &s_shared, utility::forward<Ts>(_arguments)...}
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
