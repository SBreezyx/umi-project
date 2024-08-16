#ifndef UMI_MAP_H
#define UMI_MAP_H

#include <map>
#include <ostream>

namespace umi {

template<typename K, typename V>
using map = std::map<K, V>;

template <typename K, typename V>
auto operator<<(std::ostream &os, const map<K, V> &mp) -> std::ostream& {
  if (mp.size() == 0) {
    os << "{}" ;
  } else {
    auto it = mp.begin();
    os << "{" << "{" << it->first << ", " << it->second << "}";
    ++it;
    for (const auto last = mp.end(); it != last; ++it) {
      os << ", {" << it->first << ", " << it->second << "}";
    }
    os << "}";
  }

  return os;
}

}

#endif // UMI_MAP_H
