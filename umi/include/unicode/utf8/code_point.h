#ifndef UMI_UTF8_CODE_POINT_H
#define UMI_UTF8_CODE_POINT_H

#include <unicode/types.h>

#include <ostream>


namespace utf8 {
class code_point {
public:
  code_point(const char8_t *code_unit = nullptr);


  code_point(char code_unit);


  auto operator==(char8_t r) const -> bool {
    return code_unit_ == r;
  }


  [[nodiscard]] auto size() const -> ssize_t;


  [[nodiscard]] auto data() const -> const char8_t *;


  friend auto
  operator<<(std::ostream &os, const code_point &cp) -> std::ostream & {
    for (
      auto *first = cp.data(), *last = cp.data() + cp.size();
      first != last;
      ++first
    ) {
      os << static_cast<char>(*first);
    }

    return os;
  }


  auto operator<=>(code_point other) const -> std::strong_ordering {
    return code_unit_ <=> other.code_unit_;
  }

private:
  union {
    char32_t code_unit_;
    char8_t code_units_[4];
  };
};
}

#endif // UMI_UTF8_CODE_POINT_H
