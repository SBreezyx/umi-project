#ifndef UMI_LEXER_H
#define UMI_LEXER_H

#include <umi/parse/token.h>

#include <umi/id.h>
#include <umi/parallelism.h>
#include <umi/types/map.h>
#include <umi/types/string.h>
#include <umi/types/vector.h>

namespace umi {

struct lexed {
  vector<vector<token_type>> tokens;
  map<id_type, string> common_strings;
};

struct cursor {
  const char8_t *curr;
  const char8_t *end;
  int lineno;
  int colno;
};

auto tokenize(
  const vector<string> &source_files,
  ssize_t n_threads = THREADS_AUTO
) -> lexed;

auto tokenize(vector<cursor> &cursors, vector<token_class> &tokens) -> void;

auto tokenize(ssize_t len, cursor *cursors, token_class *tokens) -> void;

} // namespace umi

#endif // UMI_LEXER_H
