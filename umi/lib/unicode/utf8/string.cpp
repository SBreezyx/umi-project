#include "sso.h"
#include "unicode/utf8/string.h"

#include <unicode/utf8.h>

#include <compare>
#include <algorithm>

using namespace utf8;

constexpr auto start_dynamic_mode(sso_t &data, ssize_t n) -> void {
  data.ptr = new char8_t[n]{};
  data.sz = -n;
}

constexpr auto start_sso_mode(sso_t &data, ssize_t n) -> void {
  data.str[n] = '\0';
  sso_encode_size(data, n);
}

string::string(const char8_t *s, ssize_t n) {
  constexpr auto CAPACITY = sizeof(string);
  if (s) {
    n = n < 0 ? strlen(s) : n;
    char8_t *data;
    if (n >= CAPACITY) {
      start_dynamic_mode(data_, n);
      data = data_.ptr;
    } else {
      start_sso_mode(data_, n);
      data = data_.str;
    }

    std::copy_n(s, n, data);
  } else {
    start_sso_mode(data_, 0);
  }
}

string::string(const char *s, ssize_t n) : string{
  reinterpret_cast<const char8_t *>(s), n
} {}

string::string(iterator first, iterator last) : string{
    first.code_units_, std::distance(first, last)
} {}

string::string(const string &other) : string(
    reinterpret_cast<const char8_t *>(other.data()),
    other.length()
) {}

string::string(string &&other) noexcept: string{} {
  swap(other);
}

string::~string() {
  if (!in_sso_mode(data_)) {
    delete[] data_.ptr;
  }
}

auto string::operator=(const string &other) -> string & {
  if (this != &other) {
    string{other}.swap(*this);
  }

  return *this;
}

auto string::operator=(string &&other) noexcept -> string & {
  if (this != &other) {
    string{}.swap(*this);
  }

  return *this;
}

auto string::operator[](int n) const -> code_point {
  auto it = begin();
  std::advance(it, n);
  return *it;
}

[[nodiscard]] auto string::data() const -> const char8_t * {
  if (in_sso_mode(data_)) {
    return data_.str;
  } else {
    return data_.ptr;
  }
}

[[nodiscard]] auto string::size() const -> ssize_t {
  return std::distance(begin(), end());
}

[[nodiscard]] auto string::length() const -> ssize_t {
  return len(data_);
}

auto string::swap(string &other) noexcept -> void {
  std::swap(data_, other.data_);
}

auto string::begin() -> iterator {
  return iterator{data()};
}

[[nodiscard]] auto string::begin() const -> const_iterator {
  return const_iterator{data()};
}

auto string::end() -> iterator {
  return const_iterator{data() + length()};
}

[[nodiscard]] auto string::end() const -> const_iterator {
  return const_iterator{data() + length()};
}

auto utf8::operator<=>(const string &l, const string &r) -> std::strong_ordering {
  auto first1 = l.begin(), last1 = l.end();
  auto first2 = r.begin(), last2 = l.end();
  for (; (first1 != last1) && (first2 != last2); ++first1, (void) ++first2) {
    if (*first1 < *first2)
      return std::strong_ordering::less;
    if (*first2 < *first1)
      return std::strong_ordering::greater;
  }

  auto first_ended = first1 == last1;
  auto second_ended = first2 != last2;
  if (first_ended && second_ended) {
    return std::strong_ordering::equal;
  } else if (first_ended) {
    return std::strong_ordering::less;
  } else {
    return std::strong_ordering::greater;
  }
}

auto utf8::operator==(const utf8::string &l, const utf8::string &r) -> bool {
  return l <=> r == std::strong_ordering::equal;
}

