#ifndef UMI_VECTOR_H
#define UMI_VECTOR_H

#include <ostream>
#include <vector>

namespace umi {

template<typename T>
using vector = std::vector<T>;

template <typename T>
auto operator<<(std::ostream &os, const vector<T> &v) -> std::ostream & {
  if (v.size() == 0) {
    os << "{}";
  } else {
    auto it = v.begin();
    os << "{" << *it++;
    for (const auto last = v.end(); it != last; ++it) {
      os << ", " << *it;
    }
    os << "}";
  }

  return os;
}

}

#endif // UMI_VECTOR_H
