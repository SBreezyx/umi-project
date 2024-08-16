#include <umi/parse/lexer.h>

#include <umi/parallelism.h>

#include <algorithm/parallel_for.h>
#include <unicode/utf8.h>

#include <cassert>

#undef EOF

namespace umi {

using token_loc = std::pair<string, std::size_t>;
using error_loc = std::pair<int, std::size_t>;
using source_token = std::pair<int, std::size_t>;

static const auto reserved_words = map<string, token_class>{
  {"module", token_class::MODULE_KEYWORD},
  {"import", token_class::IMPORT_KEYWORD},
  {"from", token_class::FROM_KEYWORD},
  {"export", token_class::EXPORT_KEYWORD},
  {"fun", token_class::FUN_KEYWORD},
  {"int", token_class::INT_KEYWORD},
  {"const", token_class::CONST_KEYWORD},
  {"return", token_class::RETURN_KEYWORD},
  {"from", token_class::FROM_KEYWORD},
  {"if", token_class::IF_KEYWORD},
  {"else", token_class::ELSE_KEYWORD},
};

static const auto punc_mp = map<utf8::code_point, token_type>{
  {'(', token_class::LEFT_PARENS},
  {')', token_class::RIGHT_PARENS},
  {'{', token_class::LEFT_BRACE},
  {'}', token_class::RIGHT_BRACE},
  {'<', token_class::LEFT_ANGLE},
  {'>', token_class::RIGHT_ANGLE},
  {'.', token_class::PERIOD},
  {':', token_class::SINGLE_COLON},
  {';', token_class::SEMICOLON},
  {'=', token_class::SINGLE_EQUALS},
  {'+', token_class::PLUS},
  {'-', token_class::MINUS},
  {',', token_class::COMMA},
  {'?', token_class::QUESTION},
  // for completeness
  {'/', token_class::FWD_SLASH},
};


template<token_class C>
static auto merge(
  map<string, vector<source_token> > &seen,
  vector<vector<token_type> > &all_tokens,
  map<id_type, string> &shared_strings
) -> void {
  auto next_id = next_token_id<C>();
  for (const auto &[s, loc] : seen) {
    shared_strings[next_id] = s;
    for (auto [source_ix, token_ix] : loc) {
      all_tokens[source_ix][token_ix] = next_id;
    }
    next_id = next_token_id<C>(next_id);
  }
}


static auto merge_shared_strings(
  const int n_sources,
  const vector<vector<token_loc> > &all_idents,
  const vector<vector<token_loc> > &all_digits,
  vector<vector<token_type> > &all_tokens,
  map<id_type, string> &shared_strings
) -> void {
  auto seen_idents = map<string, vector<source_token> >{};
  auto seen_digits = map<string, vector<source_token> >{};

  using adhoc = std::pair<
    const vector<vector<token_loc> > &, map<string, vector<source_token> > &>;
  auto lists = std::array{
    adhoc{all_idents, seen_idents},
    adhoc{all_digits, seen_digits}
  };

  // combine across source files
  nonstd::parallel_for(2, 2, [n_sources, &lists](int first, int) {
    auto &[repeated, seen] = lists[first];
    for (auto source_ix = 0; source_ix < n_sources; ++source_ix) {
      for (const auto &[s, tok_ix] : repeated[source_ix]) {
        seen[s].emplace_back(source_ix, tok_ix);
      }
    }
  });

  // merge against class
  merge<token_class::IDENTIFIER>(seen_idents, all_tokens, shared_strings);
  merge<token_class::DIGIT_SEQUENCE>(seen_digits, all_tokens, shared_strings);
}


static
auto is_ident_char(utf8::code_point cp) -> bool {
  return utf8::isalnum(cp) || cp == '_' || cp == '$' || cp == '@' || cp == '#';
}

#if _WIN32
#warning "windows not supported yet"
static
auto tokenize_whitespace(
  string::const_iterator &cursor,
  const string::const_iterator &end_cursor,
  int &ln,
  int &col
) -> void {
  do {

  } while (cursor != end_cursor && utf8::isspace(*cursor));
}
#else
static
auto tokenize_whitespace(
  string::const_iterator &cursor,
  const string::const_iterator &end_cursor,
  int &ln,
  int &col
) -> void {
  do {
    if (*cursor == '\n') {
      ln += 1;
      col = 1;
    } else {
      col += 1;
    }
    ++cursor;
  } while (cursor != end_cursor && utf8::isspace(*cursor));
}


static auto is_newline(char8_t ch) -> bool {
  return ch == '\n';
}


static constexpr auto newline_length() -> int {
  return 1;
}

#endif

static auto tokenize(
  const string &code,
  vector<token_type> &token_buffer,
  vector<token_loc> &ident_buffer,
  vector<token_loc> &digits_buffer,
  vector<error_loc> &error_buffer
) -> void {
  const auto end = code.end();
  auto cursor = code.begin();
  auto lineno = 1;
  auto colno = 1;
  while (cursor != end) {
    auto look_ahead = *cursor;

    if (utf8::isspace(look_ahead)) {
      tokenize_whitespace(cursor, end, lineno, colno);
    }
    // decimal digits only
    else if (utf8::isdigit(look_ahead)) {
      const auto start_cursor = cursor;
      do {
        ++cursor;
      } while (utf8::isdigit(*cursor) && cursor != end);

      const auto tok_ix = token_buffer.size();
      token_buffer.push_back(token_class::DIGIT_SEQUENCE);
      digits_buffer.emplace_back(string{start_cursor, cursor}, tok_ix);
    }
    /* this must come after digit check */
    else if (is_ident_char(*cursor)) {
      const auto start_cursor = cursor;

      do {
        ++cursor;
      } while (cursor != end && is_ident_char(*cursor));

      const auto identifier = string(start_cursor, cursor);

      auto found = reserved_words.find(identifier);
      if (found != reserved_words.end()) {
        token_buffer.push_back(found->second);
      } else {
        const auto tok_ix = token_buffer.size();
        token_buffer.push_back(token_class::IDENTIFIER);
        ident_buffer.emplace_back(identifier, tok_ix);
      }
    }
    // could be a comment or division
    else if (look_ahead == '/') {
      ++cursor;
      if (cursor != end && *cursor == '/') {
        do {
          ++cursor;
        } while (cursor != end && *cursor != '\n');
      } else {
        token_buffer.push_back(token_class::FWD_SLASH);
      }
    }
    // punctuation is cool
    else if (auto punc = punc_mp.find(look_ahead); punc != punc_mp.end()) {
      ++cursor;
      token_buffer.push_back(punc->second);
    }
    // stopping tokenizing at the first bad instance may not be smart
    else {
      cursor = end;
    }
  }

  token_buffer.push_back(token_class::FILE_END);
}


auto tokenize(const vector<string> &sources, ssize_t n_threads) -> lexed {
  resolve_number_of_threads(n_threads);

  // for parallel execution, we set up buffers
  // for each thread to privately work in
  const auto n_sources = static_cast<int>(sources.size());
  auto token_buffers = vector<vector<token_type> >(n_sources);
  auto ident_buffers = vector<vector<token_loc> >(n_sources);
  auto digits_buffers = vector<vector<token_loc> >(n_sources);
  auto error_buffers = vector<vector<error_loc> >(n_sources);

  // split and start the work
  nonstd::parallel_for(n_sources, n_threads, [&](int first, int last) {
    for (auto i = first; i < last; ++i) {
      tokenize(
        sources[i],
        token_buffers[i], ident_buffers[i],
        digits_buffers[i],
        error_buffers[i]
      );
    }
  });

  // do a manual gather task of all the individual strings each worker noted.
  auto shared_strs = map<id_type, string>{};
  merge_shared_strings(
    n_sources, ident_buffers, digits_buffers,
    token_buffers, shared_strs
  );

  return {std::move(token_buffers), std::move(shared_strs)};
}

auto tokenize(
  const ssize_t len,
  cursor *cursors,
  token_class *tokens
) -> void {
  for (auto i = 0; i < len; ++i) {
    auto [curr, end, lineno, colno] = cursors[i];

    // end of file
    if (curr == end) {
      tokens[i] = FILE_END;
      continue;
    }

    // skip whitespace
    while (curr != end && utf8::isspace(*curr)) {
      if (is_newline(*curr)) {
        lineno += 1;
        colno = 1;
        curr = utf8::advance(curr, newline_length());
      } else {
        colno += 1;
        curr = utf8::advance(curr, 1);
      }
    }

    if (utf8::isdigit(*curr)) {
      // TODO: digit sequence of any base
      const auto * const start = curr;
      do {
        curr = utf8::advance(curr, 1);
      } while (curr != end && utf8::isdigit(*curr));

      tokens[i] = DIGIT_SEQUENCE;
    } else if (is_ident_char(curr)) {
      const auto *const start = curr;
      auto n = 0;
      do {
        curr = utf8::advance(curr, 1);
        n += 1;
      } while (curr != end && is_ident_char(curr));

      const auto identifier = string(start, n);

      if (
        auto found = reserved_words.find(identifier);
        found != reserved_words.end()
      ) {
        tokens[i] = found->second;
      } else {
        tokens[i] = IDENTIFIER;
      }
    }

    cursors[i] = {curr, end, lineno, colno};
  }
}

auto tokenize(vector<cursor> &cursors, vector<token_class> &tokens) -> void {
  assert(cursors.size() == tokens.size());
  const auto ssize = static_cast<ssize_t>(cursors.size());
  tokenize(ssize, cursors.data(), tokens.data());
}

}
