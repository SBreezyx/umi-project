#ifndef ALGORITHM_SEQUENCE_H
#define ALGORITHM_SEQUENCE_H

#include <iterator>

namespace nonstd {

template <typename T, typename E>
concept sequence = std::input_iterator<typename T::iterator>
  && std::is_convertible_v<std::iter_value_t<typename T::iterator>, E>;

}

#endif // ALGORITHM_SEQUENCE_H
