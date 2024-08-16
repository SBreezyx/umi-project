#include "code_point.h"

#include <unicode/utf8/code_point.h>

#include <algorithm>

using namespace utf8;

auto codepoint_size(const char8_t *ustr) noexcept -> ssize_t {
  auto sz = ssize_t{};

  auto header_byte = *ustr;
  if (header_byte < CON_BYTE) {
    sz = 1;
  } else if (header_byte < THREE_BYTE) {
    sz = 2;
  } else if (header_byte < FOUR_BYTE) {
    sz = 3;
  } else {
    sz = 4;
  }

  return sz;
}

code_point::code_point(const char8_t *code_unit) :
code_unit_{} {
  std::copy_n(code_unit, code_unit == nullptr ? 0 : codepoint_size(code_unit), code_units_);
}

code_point::code_point(char code_unit) : code_point{
  reinterpret_cast<const char8_t *>(&code_unit)
} {}


auto code_point::size() const -> ssize_t {
  return codepoint_size(code_units_);
}

auto code_point::data() const -> const char8_t * {
  return code_units_;
}
