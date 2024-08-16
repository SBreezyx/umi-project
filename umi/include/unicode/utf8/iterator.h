#ifndef UMI_UTF8_ITERATOR_H
#define UMI_UTF8_ITERATOR_H

#include <unicode/utf8/code_point.h>
#include <unicode/types.h>

#include <iterator>

namespace utf8 {

class string_view;

class string;

class iterator {
  friend string_view;
  friend string;

public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = code_point;
  using reference = void;
  using difference_type = ssize_t;

  iterator() = default;

  auto operator*() const -> code_point;

  auto operator++() -> iterator &;

  auto operator++(int) -> iterator;

  auto operator==(const iterator &) const -> bool = default;

private:
  constexpr explicit
  iterator(const char8_t *code_units) : code_units_{code_units} {}

  const char8_t *code_units_;
};

static_assert(std::input_iterator<iterator>);

}

#endif // UMI_UTF8_ITERATOR_H
