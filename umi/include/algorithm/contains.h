#ifndef ALGORITHM_CONTAINS_H
#define ALGORITHM_CONTAINS_H

#include <algorithm/sequence.h>

namespace nonstd {

template<typename T, sequence<T> S>
constexpr auto contains(const S &seq, const T &item) -> bool {
  return std::find(seq.begin(), seq.end(), item) != seq.end();
}

template<typename T, sequence<T> S>
constexpr auto index_of(const S &seq, const T &item) {
  for (auto i = 0; i < seq.size(); ++i) {
    if (seq[i] == item) {
      return i;
    }
  }

  return -1;
}

}

#endif // ALGORITHM_CONTAINS_H
