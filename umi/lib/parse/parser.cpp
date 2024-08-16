#include <umi/parse/parser.h>

#include <umi/parallelism.h>

#include <algorithm/parallel_for.h>
#include <algorithm/contains.h>

#include <stack>
#include <iostream>

#include "umi/types/vector.h"

#undef EOF

#define die(s) do { std::cout << s << std::endl; std::terminate(); } while(0)

namespace umi {

using token_iter = vector<token_type>::const_iterator;

enum parser_rule {
  program,
  module_decl,
  top_level,
  ident,
  unqualified_id,
  qualified_id,
  import_stmt,
  export_stmt,
  fun_decl,
  parameter_list,
  binding_type,
  type,
  fundamental_type,
  // struct_type,
  // union_type,
  // type_modifier,
  stmt,
  block_stmt,
  if_stmt,
  var_def,
  return_stmt,
  expr,
  primary_expr,
  postfix_unary_expr,
  call_op,
  positional_args,
  named_args,
  prefix_unary_expr,
  binary_expr,
  ternary_expr,
  literal,
};

enum parse_result {
  no_result = -1,
  ok = 0
};

static auto accept_if(
  token_iter &cursor,
  token_class expected,
  const std::string &failure_msg
) {
  if (!is_token_class(expected, *cursor)) {
    die(failure_msg);
  }

  ++cursor;
}

static auto accept(
  token_iter &cursor,
  token_class
) {
  ++cursor;
}

static auto accept_if(
  token_iter &cursor,
  token_iter end_cursor,
  token_class expected,
  const std::string &failure_msg = ""
) -> void {
  if (cursor == end_cursor) {
    die("unexpected end of input");
  } else {
    accept_if(cursor, expected, failure_msg);
  }
}

template<token_class C>
static auto cursor_is(token_iter cursor) {
  return is_token_class(C, *cursor);
}

template<parser_rule R>
static auto try_parse(
  token_iter &cursor,
  const token_iter end_cursor,
  int = 0
) -> parse_result {
  auto res = parse_result::ok;
  if constexpr (R == program) {
    // read a token
    // branch:
    //  - EOF -> end of file.
    //  - "module", etc. -> top_level
    //  - その他 -> error

    try_parse<module_decl>(cursor, end_cursor);
    while (!cursor_is<FILE_END>(cursor)) {
      try_parse<top_level>(cursor, end_cursor);
    }
  }
    //
  else if constexpr (R == module_decl) {
    if (
      cursor_is<EXPORT_KEYWORD>(cursor)
      || cursor_is<MODULE_KEYWORD>(cursor + 1)
      ) {
      if (cursor_is<EXPORT_KEYWORD>(cursor)) {
        accept(cursor, EXPORT_KEYWORD);
      }
      accept(cursor, MODULE_KEYWORD);
      try_parse<ident>(cursor, end_cursor);
      accept_if(cursor, end_cursor, SEMICOLON);
    }
  }
    //
  else if constexpr (R == top_level) {
    while (!cursor_is<FILE_END>(cursor)) {
      switch (*cursor) {
        case SEMICOLON:
          ++cursor;
          continue;
        case IMPORT_KEYWORD:
          try_parse<import_stmt>(cursor, end_cursor);
          continue;
        case EXPORT_KEYWORD:
          try_parse<export_stmt>(cursor, end_cursor);
          continue;
        case FUN_KEYWORD:
          try_parse<fun_decl>(cursor, end_cursor);
          continue;
        default:
          die("no viable alternative for: " << tokenc2str(*cursor));
      }
    }
  }
    //
  else if constexpr (R == ident) {
    try_parse<unqualified_id>(cursor, end_cursor);

    while (cursor != end_cursor && cursor_is<SINGLE_COLON>(cursor)) {
      auto cp = cursor;
      ++cp;

      if (!cursor_is<SINGLE_COLON>(cp)) {
        break;
      }

      accept(cursor, SINGLE_COLON);
      accept(cursor, SINGLE_COLON);

      try_parse<unqualified_id>(cursor, end_cursor);
    }
  }
    //
  else if constexpr (R == unqualified_id) {
    accept_if(cursor, end_cursor, IDENTIFIER, "was expecting an identifier");
  }
    //
  else if constexpr (R == import_stmt) {
    accept_if(cursor, end_cursor, IMPORT_KEYWORD);

    try_parse<unqualified_id>(cursor, end_cursor);

    while (cursor_is<COMMA>(cursor)) {
      accept(cursor, COMMA);
      try_parse<unqualified_id>(cursor, end_cursor);

      if (cursor_is<FROM_KEYWORD>(cursor)) {
        accept(cursor, FROM_KEYWORD);
        try_parse<ident>(cursor, end_cursor);
      }

      // TODO: import alias
    }

    // optional trailing comma
    if (cursor_is<COMMA>(cursor)) {
      accept(cursor, COMMA);
    }

    accept_if(cursor, end_cursor, SEMICOLON, "expected ';'");
  }
    //
  else if constexpr (R == export_stmt) {
    accept_if(cursor, end_cursor, EXPORT_KEYWORD);

    switch (*cursor) {
      case FUN_KEYWORD:
        try_parse<fun_decl>(cursor, end_cursor);
        break;
      default:
        try_parse<ident>(cursor, end_cursor);
    }
  }
    //
  else if constexpr (R == fun_decl) {
    accept_if(cursor, end_cursor, FUN_KEYWORD);

    try_parse<ident>(cursor, end_cursor);

    accept_if(cursor, end_cursor, LEFT_PARENS, "was expecting (");
    try_parse<parameter_list>(cursor, end_cursor);
    accept_if(cursor, end_cursor, RIGHT_PARENS, "was expecting )");

    // TODO: add optional throws clause
    // if (cursor_is<THROWS>(cursor))...

    if (cursor_is<MINUS>(cursor)) {
      accept(cursor, MINUS);
      accept_if(cursor, end_cursor, RIGHT_ANGLE, "was expecting ->");
      try_parse<type>(cursor, end_cursor);
    }

    // actually we can accept "=" Expr as well, but we'll get there
    accept_if(cursor, end_cursor, LEFT_BRACE, "was expecting a function body");

    while (!cursor_is<FILE_END>(cursor) && !cursor_is<RIGHT_BRACE>(cursor)) {
      try_parse<stmt>(cursor, end_cursor);
    }

    accept_if(cursor, end_cursor, RIGHT_BRACE,
              "was expecting to end the function body");
  }
    //
  else if constexpr (R == parameter_list) {
    // optional binding type
    try_parse<binding_type>(cursor, end_cursor);

    if (cursor_is<IDENTIFIER>(cursor)) {
      try_parse<unqualified_id>(cursor, end_cursor);
      accept_if(cursor, end_cursor, SINGLE_COLON);
    }

    try_parse<type>(cursor, end_cursor);
  }
    //
  else if constexpr (R == type) {
    try_parse<fundamental_type>(cursor, end_cursor);
  }
    //
  else if constexpr (R == fundamental_type) {
    accept_if(cursor, end_cursor, INT_KEYWORD);
  }
    //
  else if constexpr (R == stmt) {
    switch (*cursor) {
      case IF_KEYWORD:
        try_parse<if_stmt>(cursor, end_cursor);
        break;

      case RETURN_KEYWORD:
        try_parse<return_stmt>(cursor, end_cursor);
        break;

      case CONST_KEYWORD:
        try_parse<var_def>(cursor, end_cursor);
        break;

      case LEFT_BRACE:
        try_parse<block_stmt>(cursor, end_cursor);
        break;

      default:
        die("unimplemented statement case");
    }
  }
    //
  else if constexpr (R == binding_type) {
    switch (*cursor) {
      case CONST_KEYWORD:
        accept(cursor, CONST_KEYWORD);
        break;

      default:
        res = no_result;
        break;
    }
  }
    //
  else if constexpr (R == var_def) {
    if (try_parse<binding_type>(cursor, end_cursor)) {
      return res = no_result;
    }

    try_parse<unqualified_id>(cursor, end_cursor);

    if (cursor_is<SINGLE_COLON>(cursor)) {
      accept(cursor, SINGLE_COLON);
      try_parse<type>(cursor, end_cursor);
    }

    accept_if(cursor, end_cursor, SINGLE_EQUALS);

    try_parse<expr>(cursor, end_cursor);

    accept_if(cursor, end_cursor, SEMICOLON, "expected ;");
  }
    //
  else if constexpr (R == block_stmt) {
    if (!cursor_is<LEFT_BRACE>(cursor)) {
      return no_result;
    }

    accept(cursor, LEFT_BRACE);

    while(!cursor_is<FILE_END>(cursor) && !cursor_is<RIGHT_BRACE>(cursor)) {
      try_parse<stmt>(cursor, end_cursor);
    }

    accept_if(cursor, end_cursor, RIGHT_BRACE, "expected right brace at end of if-statement");
  }
    //
  else if constexpr (R == return_stmt) {
    accept_if(cursor, end_cursor, RETURN_KEYWORD);
    try_parse<expr>(cursor, end_cursor);
    accept_if(cursor, end_cursor, SEMICOLON);
  }
    //
  else if constexpr (R == primary_expr) {
    switch (token_class_of(*cursor)) {
      case IDENTIFIER:
        try_parse<ident>(cursor, end_cursor);
        break;
      case DIGIT_SEQUENCE:
        try_parse<literal>(cursor, end_cursor);
        break;
      default:
        die("unknown primary expression case reached");
    }
  }
    //
  else if constexpr (R == postfix_unary_expr) {
    try_parse<primary_expr>(cursor, end_cursor);

    auto has_postfix_ops = true;
    while (has_postfix_ops) {
      switch (*cursor) {
        case LEFT_PARENS:
          try_parse<call_op>(cursor, end_cursor);
          break;
        default:
          has_postfix_ops = false;
          break;
      }
    }
  }
    //
  else if constexpr (R == prefix_unary_expr) {
    switch (token_class_of(*cursor)) {
      case PLUS:
      case MINUS:
        try_parse<prefix_unary_expr>(cursor, end_cursor);
        break;

      default:
        try_parse<postfix_unary_expr>(cursor, end_cursor);
    }

  }
    //
  else if constexpr (R == binary_expr) {
    try_parse<prefix_unary_expr>(cursor, end_cursor);
    switch (token_class_of(*cursor)) {
      case RIGHT_ANGLE:
        accept(cursor, RIGHT_ANGLE);
        if (cursor_is<RIGHT_ANGLE>(cursor)) {
          // bitshift-right
          accept(cursor, RIGHT_ANGLE);
        }
        try_parse<binary_expr>(cursor, end_cursor);
        break;

      case LEFT_ANGLE:
        accept(cursor, LEFT_ANGLE);
        if (cursor_is<LEFT_ANGLE>(cursor)) {
         // bit-shift left
          accept(cursor, LEFT_ANGLE);
        }

        try_parse<binary_expr>(cursor, end_cursor);
        break;

      case MINUS:
      case PLUS:
        accept(cursor, token_class_of(*cursor));
        try_parse<binary_expr>(cursor, end_cursor);
        break;

      default:
        break;
    }
  }
    //
  else if constexpr (R == ternary_expr) {
    try_parse<binary_expr>(cursor, end_cursor);

    if (cursor_is<QUESTION>(cursor)) {
      accept(cursor, QUESTION);
      if (!cursor_is<SINGLE_COLON>(cursor)) {
        try_parse<expr>(cursor, end_cursor);
      }

      accept_if(cursor, end_cursor, SINGLE_COLON,
                "expecting colon in ternary operator");
      try_parse<expr>(cursor, end_cursor);
    }

  }
    //
  else if constexpr (R == expr) {
    switch (token_class_of(*cursor)) {
//      case THROW:
      case COMMA:
        accept(cursor, COMMA);
        try_parse<expr>(cursor, end_cursor);
        break;

      default:
        try_parse<ternary_expr>(cursor, end_cursor);
        break;
    }
  }
    //
  else if constexpr (R == call_op) {
    accept_if(cursor, end_cursor, LEFT_PARENS);

    // try parse positional args first
    // then parse named args
    try_parse<positional_args>(cursor, end_cursor);
    if (!cursor_is<RIGHT_PARENS>(cursor)) {
      try_parse<named_args>(cursor, end_cursor);
    }

    accept_if(cursor, end_cursor, RIGHT_PARENS);
  }
    //
  else if constexpr (R == positional_args) {
    if (auto cp = cursor; cursor_is<IDENTIFIER>(cp)) {
      ++cp;
      if (cursor_is<SINGLE_EQUALS>(cp)) {
        return res;
      } else {
        try_parse<expr>(cursor, end_cursor);
      }
    } else {
      try_parse<expr>(cursor, end_cursor);
    }

    while (cursor_is<COMMA>(cursor)) {
      accept(cursor, COMMA);
      if (auto cp = cursor; cursor_is<IDENTIFIER>(cp)) {
        ++cp;
        if (cursor_is<SINGLE_EQUALS>(cp)) {
          return res;
        } else {
          try_parse<expr>(cursor, end_cursor);
        }
      } else if (cursor_is<RIGHT_PARENS>(cursor)) {
        // optional trailing comma
        break;
      } else {
        try_parse<expr>(cursor, end_cursor);
      }
    }
  }
    //
  else if constexpr (R == named_args) {
    accept_if(cursor, end_cursor, IDENTIFIER);
    accept_if(cursor, end_cursor, SINGLE_EQUALS);
    try_parse<expr>(cursor, end_cursor);

    while (
      !cursor_is<FILE_END>(cursor)
      && !cursor_is<COMMA>(cursor)
      && !cursor_is<RIGHT_PARENS>(cursor)
      ) {
      accept_if(cursor, end_cursor, COMMA);
      accept_if(cursor, end_cursor, IDENTIFIER);
      accept_if(cursor, end_cursor, SINGLE_EQUALS);
      try_parse<expr>(cursor, end_cursor);
    }

    // optional trailing comma
    if (cursor_is<COMMA>(cursor)) {
      accept(cursor, COMMA);
    }
  }
    //
  else if constexpr (R == literal) {
    switch (token_class_of(*cursor)) {
      case DIGIT_SEQUENCE:
        // accept a literal int or smth
        accept(cursor, DIGIT_SEQUENCE);
        break;

      default:
        die("unimplemented literal case reached");
    }
  }
    //
  else if constexpr (R == if_stmt) {
    accept_if(cursor, end_cursor, IF_KEYWORD);
    accept_if(cursor, end_cursor, LEFT_PARENS);

    // optional init statement
    try_parse<var_def>(cursor, end_cursor);

    // if-cond
    try_parse<expr>(cursor, end_cursor);

    accept_if(cursor, end_cursor, RIGHT_PARENS);

    try_parse<stmt>(cursor, end_cursor);

    if (cursor_is<ELSE_KEYWORD>(cursor)) {
      accept(cursor, ELSE_KEYWORD);
      try_parse<stmt>(cursor, end_cursor);
    }
  }
    //
  else {
    die("unimplemented parser rule");
  }

  return res;
}

static
auto naive_parse(
  const vector<token_type> &source_tokens,
  int
) {
  const auto end_cursor = source_tokens.end();

  auto cursor = source_tokens.begin();

  try_parse<program>(cursor, end_cursor);
}


auto parse(
  const vector<vector<token_type>> &source_tokens,
  ssize_t n_threads
) -> void {
  resolve_number_of_threads(n_threads);

  const auto n_sources = static_cast<ssize_t>(source_tokens.size());
  nonstd::parallel_for(n_sources, n_threads, [&](int f, int l) {
    for (auto i = f; i < l; ++i) {
      naive_parse(source_tokens[i], 0);
    }
  });

}


}