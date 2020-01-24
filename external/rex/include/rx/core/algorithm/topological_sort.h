#ifndef RX_CORE_ALGORITHM_TOPOLOGICAL_SORT_H
#define RX_CORE_ALGORITHM_TOPOLOGICAL_SORT_H
#include "rx/core/map.h"
#include "rx/core/set.h"
#include "rx/core/vector.h"

namespace rx::algorithm {

// # Topological Sort
//
// Really fast O(V+E), generic topological sorting using unordered hasing
// data structures.
//
// K must satisfy hashing and comparison with operator== for this to be used.
//
// Add nodes with |add(_key)|
// Add dependencies with |add(_key, _dependency)|
template<typename K>
struct topological_sort {
  topological_sort();
  topological_sort(memory::allocator* _allocator);

  struct result {
    vector<K> sorted; // sorted order of nodes
    vector<K> cycled; // problem nodes that form cycles
  };

  bool add(const K& _key);
  bool add(const K& _key, const K& _dependency);

  result sort();

  void clear();

protected:
  struct relations {
    relations(memory::allocator* _allocator);
    rx_size dependencies;
    set<K> dependents;
  };

  memory::allocator* m_allocator;
  map<K, relations> m_map;
};

template<typename K>
inline topological_sort<K>::topological_sort()
  : topological_sort{&memory::g_system_allocator}
{
}

template<typename K>
inline topological_sort<K>::topological_sort(memory::allocator* _allocator)
  : m_allocator{_allocator}
  , m_map{_allocator}
{
}

template<typename K>
inline bool topological_sort<K>::add(const K& _key) {
  if (m_map.find(_key)) {
    return false;
  }
  return m_map.insert(_key, {m_allocator}) != nullptr;
}

template<typename K>
inline bool topological_sort<K>::add(const K& _key, const K& _dependency) {
  // Cannot be a dependency of one-self.
  if (_key == _dependency) {
    return false;
  }

  // Dependents of the dependency.
  {
    auto find{m_map.find(_dependency)};
    if (!find) {
      find = m_map.insert(_dependency, {m_allocator});
    }
    auto& dependents{find->dependents};

    // Already a dependency.
    if (dependents.find(_key)) {
      return true;
    }

    dependents.insert(_key);
  }

  // Dependents of the key.
  {
    auto find{m_map.find(_key)};
    if (!find) {
      find = m_map.insert(_key, {m_allocator});
    }
    auto& dependencies{find->dependencies};

    // Another reference for this dependency.
    dependencies++;
  }

  return true;
}

template<typename K>
inline typename topological_sort<K>::result topological_sort<K>::sort() {
  // Make a copy of the map because the sorting is destructive.
  auto map{m_map};

  vector<K> sorted{m_allocator};
  vector<K> cycled{m_allocator};

  // Each key that has no dependencies can be put in right away.
  map.each_pair([&](const K& _key, const relations& _relations) {
    if (!_relations.dependencies) {
      sorted.push_back(_key);
    }
  });

  // Check dependents of the ones with no dependencies and store for each
  // resolved dependency.
  sorted.each_fwd([&](const K& _root_key) {
    map.find(_root_key)->dependents.each([&](const K& _key) {
      if (!--map.find(_key)->dependencies) {
        sorted.push_back(_key);
      }
    });
  });

  // When there's remaining dependencies of a relation then we've formed a cycle.
  map.each_pair([&](const K& _key, const relations& _relations) {
    if (_relations.dependencies) {
      cycled.push_back(_key);
    }
  });

  return {utility::move(sorted), utility::move(cycled)};
}

template<typename T>
inline void topological_sort<T>::clear() {
  m_map.clear();
}

template<typename T>
inline topological_sort<T>::relations::relations(memory::allocator* _allocator)
  : dependencies{0}
  , dependents{_allocator}
{
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_TOPOLIGCAL_SORT_H
