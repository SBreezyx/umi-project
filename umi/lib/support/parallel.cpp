#include <umi/parallelism.h>

#include <thread>

namespace umi {

auto resolve_number_of_threads(ssize_t &n_threads) -> void {
  if (n_threads <= THREADS_AUTO) {
    n_threads = static_cast<int>(std::thread::hardware_concurrency());
  }
}


}