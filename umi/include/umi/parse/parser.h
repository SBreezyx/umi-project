#ifndef UMI_PARSER_H
#define UMI_PARSER_H

#include <umi/parse/token.h>

#include <umi/types/vector.h>
#include <umi/types/string.h>
#include <umi/parallelism.h>

#include <algorithm/ssize.h>

namespace umi {

using nonstd::ssize_t;

enum parse_error_class {

};

struct error_loc {
  int lineno;
  int colno;
  parse_error_class err;
};


auto parse(
  const vector<vector<token_type>> &source_tokens,
  ssize_t n_threads = THREADS_AUTO
) -> void;


/**
 * Parse the list of code sources in parallel.
 * Parsing is done in a recursive-descent style of parsing.
 * Each parsing function examines the next token and narrows down the
 * list of possible productions to apply according to the token input stream.
 * <br /> <br />
 * This function assumes the grammar is unambiguous and will probably not
 * function as intended if the grammar has unintended ambiguities.
 * <br /><br />
 * For the time being, I want to (re-)get this style of parsing implemented
 * so I can continue to design how the rest of the compiler will go about
 * performing semantic analysis and eventual LLVM IR generation.
 *
 * @param sources   the code sources to parse.
 * @param n_threads the number of threads to use while parsing. <br />
 *                  Defaults to the number of physical CPU cores.
 */
auto parse(
  const vector<string> &sources,
  ssize_t n_threads = THREADS_AUTO
) -> void;


}

#endif // UMI_PARSER_H