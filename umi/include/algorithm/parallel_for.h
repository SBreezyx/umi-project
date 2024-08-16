#ifndef UMI_PARALLEL_FOR_H
#define UMI_PARALLEL_FOR_H

#include <algorithm/ssize.h>
#include <algorithm/partition_range.h>

#include <thread>


namespace nonstd {

/**
 *
 * Split the range [0, @p n) into (roughly) equal sized chunks and use
 * @p worker to process each parition in parallel.
 *
 * @tparam Fn Callback type of the worker function
 * @tparam Args Argument types to pass to the worker function
 * @param n the size of the range to partition into indices
 * @param n_threads the number of threads to allocate to each partition
 * @param worker the function to use for parallel processing
 * @param args the args to pass to the worker function
 */
template<typename Fn, typename... Args>
auto parallel_for(
  const ssize_t n,
  const ssize_t n_threads,
  Fn worker,
  Args &&... args
) -> void requires std::is_invocable_v<Fn, int, int, Args...> {
  auto threads = std::vector<std::thread>{};
  for (auto [f, l] : partition_range(n, n_threads)) {
    threads.emplace_back([f, l, &worker, &args...] {
      worker(f, l, std::forward<Args>(args)...);
    });
  }

  for (auto &t : threads) {
    t.join();
  }
}

}

#endif // UMI_PARALLEL_FOR_H
