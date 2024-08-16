#ifndef UMI_PARTITION_RANGE_H
#define UMI_PARTITION_RANGE_H

#include <concepts>
#include <vector>

namespace nonstd {
struct partition_result {
  int first;
  int last;
};

constexpr auto partition_range(
  const int sz,
  const int n_windows
) -> std::vector<partition_result> {
  auto partitions = std::vector<partition_result>{};

  auto ix = 0;
  if (sz >= n_windows) {
    const auto step = sz / n_windows;
    while (ix + step < sz) {
      const auto next = ix + step;
      partitions.push_back({ix, next});
      ix = next;
    }
  }

  partitions.push_back({ix, sz});

  return std::move(partitions);
}

}

#endif // UMI_PARTITION_RANGE_H
