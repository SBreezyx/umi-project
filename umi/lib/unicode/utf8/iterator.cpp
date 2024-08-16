#include <unicode/utf8/iterator.h>

#include <unicode/utf8/algorithm.h>

using namespace utf8;

auto iterator::operator*() const -> code_point {
  return code_point{code_units_};
}

auto iterator::operator++() -> iterator & {
  code_units_ = advance(code_units_, 1);

  return *this;
}

auto iterator::operator++(int) -> iterator {
  auto ret = *this;
  ++*this;
  return ret;
}

