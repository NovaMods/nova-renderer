#ifndef RX_CORE_TRAITS_TYPE_IDENTITY_H
#define RX_CORE_TRAITS_TYPE_IDENTITY_H

namespace rx::traits {

template<typename T>
struct type_identity {
  using type = T;
};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_TYPE_IDENTITY_H
