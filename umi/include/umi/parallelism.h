#ifndef UMI_PARALLELISM_H
#define UMI_PARALLELISM_H

#include <algorithm/ssize.h>

namespace umi {

using nonstd::ssize_t;

constexpr auto THREADS_AUTO = 0;

auto resolve_number_of_threads(ssize_t &n_threads) -> void;

}

#endif //UMI_PARALLELISM_H
