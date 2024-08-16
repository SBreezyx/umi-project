#include <algorithm/partition_range.h>

#include <catch2/catch_test_macros.hpp>

namespace nonstd {
auto operator==(partition_result a, partition_result b) -> bool {
  return a.first == b.first && a.last == b.last;
}
}

TEST_CASE("can i partition a range with a big window") {
//  const auto ints = std::vector{1, 2, 3};

  const auto expected = std::vector{
    nonstd::partition_result{0, 3}
  };

  const auto actual = nonstd::partition_range(3, 4);

  CHECK(actual[0] == expected[0]);
}

TEST_CASE("can i partition a range with an equal window") {
//  const auto ints = std::vector{1, 2, 3};

  const auto b = 0;
  const auto expected = std::vector{
    nonstd::partition_result{b, b + 1},
    nonstd::partition_result{b + 1, b + 2},
    nonstd::partition_result{b + 2, b + 3},
  };

  const auto actual = nonstd::partition_range(3, 3);

  CHECK(actual[0] == expected[0]);
  CHECK(actual[1] == expected[1]);
  CHECK(actual[2] == expected[2]);
}

TEST_CASE("can i partition a range with a small window") {
  const auto ints = std::vector{1, 2, 3, 4, 5};

  const auto b = 0;
  const auto expected = std::vector{
    nonstd::partition_result{b, b + 2},
    nonstd::partition_result{b + 2, b + 4},
    nonstd::partition_result{b + 4, b + 5},
  };

  const auto actual = nonstd::partition_range(5, 2);

  CHECK(actual[0] == expected[0]);
  CHECK(actual[1] == expected[1]);
  CHECK(actual[2] == expected[2]);
}