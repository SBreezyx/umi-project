#ifndef UMI_INSERT_IF_H
#define UMI_INSERT_IF_H

#include <concepts>
#include <vector>

namespace nonstd {

template <typename T, typename Fn>
auto insert_if(std::vector<T> &v, const T &elem, Fn pred) -> void
requires std::is_invocable_r_v<bool, Fn, T> {
  if (pred(elem)) {
    v.push_back(elem);
  }

}

}

#endif //UMI_INSERT_IF_H
