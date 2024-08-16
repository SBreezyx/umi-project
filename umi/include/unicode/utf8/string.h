#ifndef UMI_UTF8_STRING_H
#define UMI_UTF8_STRING_H

#include <unicode/utf8/code_point.h>
#include <unicode/utf8/iterator.h>

#include <compare>

namespace utf8 {
  union sso_t {
    struct {
      char8_t *ptr;
      ssize_t sz;
    };
    struct {
      char8_t str[sizeof(char8_t *) + sizeof(ssize_t)];
    };
  };

  class string {
  public:
    using iterator = utf8::iterator;
    using const_iterator = iterator;

    string(const char8_t *s = nullptr, ssize_t n = -1);

    string(const char *s, ssize_t n = -1);

    string(iterator first, iterator last);

    string(const string &other);

    string(string &&other) noexcept;

    ~string();

    auto operator=(const string &other) -> string &;

    auto operator=(string &&other) noexcept -> string &;

    auto operator[](int n) const -> code_point;

    [[nodiscard]] auto data() const -> const char8_t *;

    [[nodiscard]] auto size() const -> ssize_t;

    [[nodiscard]] auto length() const -> ssize_t;

    auto swap(string &other) noexcept -> void;

    auto begin() -> iterator;

    [[nodiscard]] auto begin() const -> const_iterator;

    auto end() -> iterator;

    [[nodiscard]] auto end() const -> const_iterator;

    friend auto
    operator<=>(const string &l, const string &r) -> std::strong_ordering;

    friend auto
    operator<<(std::ostream &os, const string &sv) -> std::ostream & {
      os << "\"";
      for (const auto &e: sv) {
        os << e;
      }

      os << "\"";

      return os;
    }

  private:
    sso_t data_;
  };

  auto operator<=>(const string &l, const string &r) -> std::strong_ordering;
  auto operator==(const string &l, const string &r) -> bool;
}

#endif // UMI_UTF8_STRING_H
