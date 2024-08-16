#include "code_point.h"

#include <unicode/utf8/algorithm.h>


auto utf8::strlen(const char8_t *s) -> int {
  auto n = 0;

  if (s) {
    while (*s != '\0') ++n, ++s;
  }
  return n;
}


auto utf8::advance(const char8_t *str, ssize_t n_steps) -> const char8_t * {
  for (auto i = 0; i < n_steps; ++i) {
    str += codepoint_size(str);
  }

  return str;
}


auto utf8::isalnum(const code_point &cp) -> bool {
  return std::isalnum(cp.data()[0]);
}


auto utf8::isspace(const code_point &cp) -> bool {
  return isspace(*cp.data());
}


auto utf8::isspace(const char8_t ch) -> bool {
  return std::isspace(ch);
}


auto utf8::isdigit(const code_point &cp) -> bool {
  return isdigit(*cp.data());
}


auto utf8::isdigit(const char8_t ch) -> bool {
  return std::isdigit(ch);
}


auto utf8::tolower(const code_point &cp) -> code_point {
  auto c = std::tolower(cp.data()[0]);
  return code_point{reinterpret_cast<char8_t *>(&c)};
}


auto utf8::isalpha(const code_point &cp) -> bool {
  return std::isalpha(cp.data()[0]);
}
