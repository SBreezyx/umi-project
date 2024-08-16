#include <umi/parse/lexer.h>

#include <catch2/catch_all.hpp>

#include <iostream>
#include <ranges>
#include <vector>

using namespace umi;


TEST_CASE("something parallel", "[parallel]") {
  const auto sources = std::vector<utf8::string>{R"(
  // hi
  import fib;

  fun main() -> int {
      const num0: int = fib::fib(0);

      return num0;
  }

  export main;
  )", R"(
  // hi
  import fib;

  fun main() -> int {
      const num1: int = fib::fib(1);

      return num1;
  }

  export main;
  )", R"(
  // hi
  import fib;

  fun main() -> int {
      const num2: int = fib::fib(2);

      return num2;
  }

  export main;
  )"};

  auto res = tokenize(sources, 2);


  const auto expected_tokens = std::vector<token_class>{
    IMPORT_KEYWORD,
    IDENTIFIER,
    SEMICOLON,
    FUN_KEYWORD,
    IDENTIFIER,
    LEFT_PARENS,
    RIGHT_PARENS,
    MINUS,
    RIGHT_ANGLE,
    INT_KEYWORD,
    LEFT_BRACE,
    CONST_KEYWORD,
    IDENTIFIER,
    SINGLE_COLON,
    INT_KEYWORD,
    SINGLE_EQUALS,
    IDENTIFIER,
    SINGLE_COLON,
    SINGLE_COLON,
    IDENTIFIER,
    LEFT_PARENS,
    DIGIT_SEQUENCE,
    RIGHT_PARENS,
    SEMICOLON,
    RETURN_KEYWORD,
    IDENTIFIER,
    SEMICOLON,
    RIGHT_BRACE,
    EXPORT_KEYWORD,
    IDENTIFIER,
    SEMICOLON
  };

  for (const auto &toks : res.tokens) {
    CHECK(std::ranges::equal(expected_tokens, toks, is_token_class));
  }

  const auto expected_strings = std::vector<umi::string>{
    "main",
    "num0",
    "num1",
    "num2",
    "fib",
    "0",
    "1",
    "2",
  };
  for (auto &[k, v] : res.common_strings) {

    CHECK(std::ranges::any_of(expected_strings, [&v](const auto &e) {
      return e == v;
    }));
  }
  std::cout << res.common_strings << std::endl;
}
