#include <umi/parse/lexer.h>
#include <umi/parse/parser.h>

#include <algorithm/parallel_for.h>
#include <umi/types/string.h>
#include <umi/types/vector.h>

#include <algorithm>


/**
 * THINKPAD
 *
 * the grammar is not LL(1).... what to do?
 * - have disambiguation states and don't always throw away the most recent token
 * - speculative execution style parsing
 *  - having multiple parsing trees active _at once_
 *  - one of 3 outcomes:
 *    1. the parse trees resolve into a single alternative (ideal)
 *    2. no parse tree resolves (error)
 *    3. more than 1 parse tree resolves (ambiguous)
 *  - with careful language construction, (3) can be avoided.
 *  - the only question is then how to:
 *    1. NOT fail at the first sign of an error
 *    2. but then how to detect that an actual error (2) happens??
 *    3. when it comes to building semantic information of the program,
 *       how to disambiguate, track, and eventually commit the data in the face
 *       of multiple active parse trees?
 *       - don't commit information until we know for sure that only one parse tree exists
 *    parse trees end when there is an error.
 *    we could keep track of which module IDs are in the error state!
 *    then how to flush the errors once a parse tree resolves?
 *    - we need to know which rules create the split
 *    - once we reach the end of the rule, then we flush any tentative errors
 *    then, how do we know that no parse tree has resolved?
 *      - when setting up for the next round of the update loop, we see all the module IDs.
 *      - if a module ID is missing, it means that nothing resolved!
 *      - that's when we can flush errors and enter the skip-token state to resync
 *
 *
 */
namespace umi {

/**
 * each parse rule is an amalgamation of one or more productions.
 * It can either be the start of a new rule, or the next logical bit to match in a production.
 * As such, at most one parse_rule is assigned as the place to move on to during parsing.
 */
enum parse_rules : unsigned {
  err_unexpected_token,

  program,

  module_decl,
  module_decl_match_module,
  /* module_decl_match_id, */
  module_decl_match_semicolon,

  export_stmt,

  ident,
  unqualified_id,
  qualified_id,


  fun_proto,
  import_stmt,
  empty_stmt,
  eof,
  n_rules = eof // must be last.
};


auto other(
  const vector<int> &module_ids,
  const vector<token_class> &next_tokens,
  vector<std::pair<int, parse_rules> > &next_stages
) -> void {}


auto matcher_program(
  const vector<int> &module_ids,
  const vector<token_class> &next_tokens,
  vector<std::pair<int, parse_rules>> &next_stages
) -> void {
  for (const auto m_id : module_ids) {
    switch (next_tokens[m_id]) {
      case FILE_START:
        // what to do??? - parallel parsing.
        next_stages.emplace_back(m_id, module_decl);
        next_stages.emplace_back(m_id, export_stmt);
        next_stages.emplace_back(m_id, import_stmt);
        next_stages.emplace_back(m_id, fun_proto);
        next_stages.emplace_back(m_id, eof);
        break;

      default:
        next_stages.emplace_back(m_id, err_unexpected_token);
        break;
    }
  }
}


auto matcher_module_decl(
  const vector<int> &module_ids,
  const vector<token_class> &next_tokens,
  vector<std::pair<int, parse_rules>> &next_stages
) -> void {
  for (const auto m_id : module_ids) {
    switch (next_tokens[m_id]) {
      case EXPORT_KEYWORD:
        next_stages.emplace_back(m_id, module_decl_match_module);
        break;

      case MODULE_KEYWORD:
        // need to install where to go
        next_stages.emplace_back(m_id, ident);
        break;

      default:
        next_stages.emplace_back(m_id, err_unexpected_token);
        break;
    }
  }
}

auto matcher_ident(
  const vector<int> &module_ids,
  const vector<token_class> &next_tokens,
  const vector<parse_rules> &parse_stack,
  vector<std::pair<int, parse_rules>> &next_stages
) -> void {
  for (const auto m_id : module_ids) {
    switch(next_tokens[m_id]) {
      case SINGLE_COLON:
        break;

      default:
        // need to pop off where to return to (end of rule)
        next_stages.emplace_back(m_id, parse_stack[m_id]);
        break;
    }
  }
}


constexpr auto parse_fns = [] {
  auto fns = std::array<decltype(matcher_program) *, n_rules>{};

  fns[err_unexpected_token] = other;
  fns[program] = matcher_program;
  fns[module_decl] = matcher_module_decl;
  fns[fun_proto] = other;
  fns[export_stmt] = other;
  fns[import_stmt] = other;
  fns[empty_stmt] = other;

  return fns;
}();
static_assert(parse_fns.size() == n_rules);

constexpr auto tokenizable = std::array {
  module_decl,
  export_stmt,
  fun_proto,
};

// modules are ID'd by their place in @p sources
// initially all module IDs are in the "program" rule.
// as parsing progresses, the module IDs are moved around the various
// parser rule/subrule lists.
// at the beginning of the update loop, the next token for each module is calculated
// this token is then placed into a lookaside buffer for each rule-list.
// then each list is processed in parallel.
auto parse(
  const vector<string> &sources,
  ssize_t n_threads
) -> void {
  resolve_number_of_threads(n_threads);

  const auto n_sources = sources.size();

  auto module_ids = std::array<vector<int>, n_rules>{};
  auto cursors = std::array<vector<cursor>, n_rules>{};
  auto tokens = std::array<vector<token_class>, n_rules>{};
  for (auto i = 0; i < n_sources; ++i) {
    cursors[program].push_back({
      sources[i].data(), sources[i].data() + sources[i].length(), 1, 1
    });
    module_ids[program].push_back(i);
    tokens[program].push_back(FILE_START);
  }

  auto next_stages = std::array<
    vector<std::pair<int, parse_rules> >, n_rules
  >{};
  auto next_module_ids = std::array<vector<int>, n_rules>{};
  auto next_cursors = std::array<vector<cursor>, n_rules>{};

  while (module_ids[eof].size() != n_sources) {
    nonstd::parallel_for(n_rules, n_threads,
       [&](const unsigned first, const unsigned last) {
         for (auto ix = first; ix != last; ++ix) {
           // pre-transition hooks called
           parse_fns[ix](module_ids[ix], tokens[ix], next_stages[ix]);
           // post-transition hooks called
         }
       }
    );

    // TODO: here, errors in parsing need to be addressed

    // prepare input lists for next round
    for (auto i = static_cast<unsigned>(program); i < n_rules; ++i) {
      module_ids[i].clear();
    }

    // transition all module ids into their next productions list for the next round
    // merge all local work into global next stage
    for (const auto &production_list : next_stages) {
      for (const auto [mod_id, prod] : production_list) {
        module_ids[prod].push_back(mod_id);
      }
    }
  }
}


}


// 13 base 10
// 1 * 10 + 3 * 1
// 1 * 10^1 + 3 * 10^0
// 1 * 2^3 + 1 * 2^2 + 0 * 2^1 + 1 * 2^0
// 1101 base 2

// 12 * 3

// 12 +
// 12 +
// 12
// --
// 36

// 1101 *
// 0011
// ----
