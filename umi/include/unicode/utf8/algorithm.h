#ifndef UMI_UTF8_ALGORITHM_H
#define UMI_UTF8_ALGORITHM_H

#include <unicode/utf8/string.h>
#include <unicode/utf8/string_view.h>

#include <unicode/types.h>


namespace utf8 {
auto strlen(const char8_t *s) -> int;


/**
* \brief Steps n_steps many characters through a UTF-8 string s
* \param s       A properly formatted UTF-8 string
* \param n_steps The number of CODE POINTS (colloquially, characters) to advance
* \return        a new offset into the original string that has been advanced between 1 - 4 bytes
*/
auto advance(const char8_t *str, ssize_t n_steps) -> const char8_t *;


template<typename T>
concept stringy = std::is_same_v<T, string> || std::is_same_v<T, string_view>;


template<stringy Str>
constexpr auto substr(Str sv, ssize_t start, ssize_t end = -1) -> Str {
  end = end == -1 ? sv.the_size() : end;

  const auto s = advance(sv.data(), start);
  const auto e = advance(s, end - start);

  return Str{s, e - s};
}


auto isalpha(const code_point &cp) -> bool;


auto isalnum(const code_point &cp) -> bool;


auto isspace(const code_point &cp) -> bool;


auto isspace(char8_t ch) -> bool;


auto isdigit(const code_point &cp) -> bool;


auto isdigit(char8_t ch) -> bool;


auto tolower(const code_point &cp) -> code_point;
}

#endif // UMI_UTF8_ALGORITHM_H
