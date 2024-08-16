#ifndef UMI_UTF8_STRING_VIEW_H
#define UMI_UTF8_STRING_VIEW_H

#include <unicode/utf8/iterator.h>
#include <unicode/utf8/code_point.h>

#include <string>

namespace utf8 {
  class string_view {
  public:
    using iterator = iterator;
    using const_iterator = iterator;
    // using reverse_iterator = std::reverse_iterator<iterator>;
    // using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    string_view(const char8_t *s = nullptr, ssize_t n = -1);

    explicit string_view(const char *s, ssize_t n = -1);

    explicit string_view(const std::string &str);

    explicit operator std::string() const;

    auto operator[](int n) const -> code_point;

    // get the number of CODE POINTS
    [[nodiscard]] auto size() const -> ssize_t;

    // get the number of CODE UNITS
    [[nodiscard]] auto length() const -> ssize_t;

    [[nodiscard]] auto data() const -> const char8_t *;

    auto begin() -> iterator;

    [[nodiscard]] auto begin() const -> const_iterator;

    auto end() -> iterator;

    [[nodiscard]] auto end() const -> const_iterator;

    friend
    auto operator==(const string_view &l, const string_view &r) noexcept -> bool {
      return std::equal(l.data(), l.data() + l.length(), r.data());
    }

    friend
    auto operator<<(std::ostream &os, const string_view &sv) -> std::ostream & {
      for (const auto &e: sv) {
        os << e;
      }

      return os;
    }

  private:
    const char8_t *code_units_;
    ssize_t n_code_units_;
  };
}

#endif // UMI_UTF8_STRING_VIEW_H
