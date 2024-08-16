#include <unicode/utf8/string_view.h>

#include <cstring>

using namespace utf8;

string_view::string_view(
    const char8_t *s, ssize_t n
) : code_units_{s}, n_code_units_{n} {}

string_view::string_view(const char *s, ssize_t n) : string_view{
    reinterpret_cast<const char8_t *>(s),
    n < 0 ? static_cast<ssize_t>(strlen(s)) : n
} {}

string_view::string_view(const std::string &str) : string_view{
    str.data(), static_cast<ssize_t>(str.size())
} {}

string_view::operator std::string() const {
  return std::string{
      reinterpret_cast<const char *>(data()),
      static_cast<std::string::size_type>(n_code_units_)
  };
}

auto string_view::operator[](int n) const -> code_point {
  auto it = begin();
  std::advance(it, n);
  return *it;
}

// get the number of CODE POINTS
[[nodiscard]] auto string_view::size() const -> ssize_t {
  return std::distance(begin(), end());
}

// get the number of CODE UNITS
[[nodiscard]] auto string_view::length() const -> ssize_t {
  return n_code_units_;
}

[[nodiscard]] auto string_view::data() const -> const char8_t * {
  return code_units_;
}

auto string_view::begin() -> iterator {
  return iterator{code_units_};
}

[[nodiscard]] auto string_view::begin() const -> const_iterator {
  return const_iterator{code_units_};
}

auto string_view::end() -> iterator {
  return const_iterator{code_units_ + n_code_units_};
}

[[nodiscard]] auto string_view::end() const -> const_iterator {
  return const_iterator{code_units_ + n_code_units_};
}