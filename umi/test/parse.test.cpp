#include <umi/parse/parser.h>

//#include <catch2/catch_test_macros.hpp>

#include <array>
#include <iostream>
#include <map>
#include <stack>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <vector>

using namespace std;


#include <concepts>

template <typename T>
concept trivial = is_trivial_v<T>;

class acton24 {
public:

  template <uint32_t ComponentName, trivial Type>
  auto create_component(this acton24 *self) -> void;

  template <uint32_t ComponentName>
  auto delete_component(this acton24 *self) -> void;



private:


};




template<typename T>
auto operator<<(ostream &os, const vector<T> &v) -> ostream & {
  os << "[";
  if (!v.empty()) {
    auto i = 0;
    os << v[i];
    for (const auto end = v.size(); i < end; ++i) { os << " " << v[i]; }
  }

  os << "]";
  return os;
}

constexpr auto WS_CHARS = array{'\f', '\t', ' ', '\v'};


using LexerStateType = unsigned char;
enum LexerState : LexerStateType {
  T_START = 0,

  // -- identifiers (left-factored to remove common prefix)
  T_IDENT_OR_C,
  T_IDENT_OR_CO,
  T_IDENT_OR_CON,
  T_IDENT_OR_CONS,
  T_IDENT_OR_CONST,
  T_CONST,
  T_IDENT_OR_E,
  T_IDENT_OR_EL,
  T_IDENT_OR_ELS,
  T_IDENT_OR_ELSE,
  T_ELSE,
  T_IDENT_OR_EX,
  T_IDENT_OR_EXP,
  T_IDENT_OR_EXPO,
  T_IDENT_OR_EXPOR,
  T_IDENT_OR_EXPORT,
  T_EXPORT,
  T_IDENT_OR_F,
  T_IDENT_OR_FR,
  T_IDENT_OR_FRO,
  T_IDENT_OR_FROM,
  T_FROM,
  T_IDENT_OR_FU,
  T_IDENT_OR_FUN,
  T_FUN,
  T_IDENT_OR_I,
  T_IDENT_OR_IF,
  T_IF,
  T_IDENT_OR_IN,
  T_IDENT_OR_INT,
  T_INT,
  T_IDENT_OR_IM,
  T_IDENT_OR_IMP,
  T_IDENT_OR_IMPO,
  T_IDENT_OR_IMPOR,
  T_IDENT_OR_IMPORT,
  T_IMPORT,
  T_IDENT_OR_M,
  T_IDENT_OR_MO,
  T_IDENT_OR_MOD,
  T_IDENT_OR_MODU,
  T_IDENT_OR_MODUL,
  T_IDENT_OR_MODULE,
  T_MODULE,
  T_IDENT_OR_R,
  T_IDENT_OR_RE,
  T_IDENT_OR_RET,
  T_IDENT_OR_RETU,
  T_IDENT_OR_RETUR,
  T_IDENT_OR_RETURN,
  T_RETURN,
  T_IDENT,
  // -- end identifiers
  // -- numbers
  T_NUMBER_0,   // 0\S
  T_NUMBER_HEX, // 0x
  T_NUMBER_OCT, // 0\d
  T_NUMBER_BIN, // 0b
  T_NUMBER_DEC, // [1-9]
  // TODO fractions, exponents, etc.
  // -- end numbers
  // -- (select) operators
  T_PLUS_OR_INC_OR_ASSIGN,
  T_PLUS,
  T_INC,
  T_PLUS_ASSIGN,
  T_MINUS_OR_DEC_OR_RARROW_OR_ASSIGN,
  T_MINUS,
  T_DEC,
  T_RARROW,
  T_MINUS_ASSIGN,
  T_MUL_OR_ASSIGN,
  T_MUL,
  T_MUL_ASSIGN,
  T_DIV_OR_COMMENT_OR_ASSIGN,
  T_DIV,
  T_MULTINE_COMMENT,
  T_EOL_COMMENT,
  T_DIV_ASSIGN,
  T_LT_OR_BSHIFT_L,
  T_LT,
  T_GT,
  T_COMMA,
  T_EQ_OR_ASSIGN,
  T_EQ,
  T_ASSIGN,
  T_SEMICOLON,
  T_COLON_OR_SCOPE,
  T_COLON,
  T_SCOPE,
  T_QUESTION,
  // -- end operators
  // -- punctuation
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  // -- end punctuation
  // -- whitespace
  T_CR_OR_CRLF,
  T_LF_OR_LFCR,
  T_CR,
  T_LF,
  T_CRLF,
  T_LFCR,
  // -- end whitespace
  N_LEX_STATES,
};


using ParserStateType = unsigned char;
enum ParserState : ParserStateType {
  NT_SOURCE_FILE_ENTER = 0,
  NT_SOURCE_FILE_BRANCH,
  NT_TOP_LEVEL,
  P_ERR,

  // these must be last
  NT_EOF,
  N_PARSE_STATES = NT_EOF,
};


struct LexerTable {
  struct {
    uint32_t n_tus;
    uint32_t *tu_ids;
    const char8_t **code_base;
    uint32_t *end_offsets;

    // can be optimised further
    uint32_t *token_start_offsets;
    uint32_t *char_offsets;
    uint32_t *line_nos;
    uint32_t *col_nos;
  } partitions[N_LEX_STATES];

  struct {
    uint32_t n_tus_total;
    uint32_t *tu_ids_total;
    struct {
      LexerState partition;
      // ... padding ...
      uint32_t index;
    } *tu_metadata;
  } metadata;
};

struct ReturnErrorPair {
  ParserState on_return;
  ParserState on_err;
};

struct ParserTable {
  struct {
    uint32_t n_tus;
    uint32_t *tu_ids;
    uint32_t *token_entities;
    stack<ReturnErrorPair> *return_stacks;
    stack<uint32_t> *ast_entities;
  } partitions[N_PARSE_STATES];

  struct  {
    uint32_t n_tus_total;
    uint32_t *tu_ids_total;
    struct MD {
      LexerState partition;
      // ... padding ...
      uint32_t index;
    } *tu_metadata;
  } metadata;
};



using unique_lex_table = unique_ptr<LexerTable, void(*)(LexerTable *)>;
auto destroy_lexer_table(LexerTable *lt) -> void {

}

auto init_lexer_table(
  const vector<vector<char>> &source_files,
  LexerState initial_state = T_START
) -> unique_lex_table {
  auto ptr = unique_lex_table(new LexerTable{}, &destroy_lexer_table);
  if (ptr) {
    const auto tu_count = static_cast<uint32_t>(source_files.size());

    auto *md = &ptr->metadata;
    auto *record = &ptr->partitions[initial_state];

    md->n_tus_total = tu_count;
    md->tu_ids_total = new uint32_t[tu_count];
    auto &f = typeid(ParserTable::metadata);

//    md->tu_metadata = new LexerTable:: [tu_count];


  }


  return ptr;
}



namespace util {
template<integral Int>
auto cantor_pair(Int x, Int y) {
  // 1/2 * (x + y) + (x + y + 1) + y
  // x/2 + y/2 + 2x/2 + 3y/2 + 1 + 2y/2
  // (3x + 5y + 2)/2

  return (x * x + x + 2 * x * y + 3 * y + y * y) >> 1;
}


template<integral Int>
auto hash_triple(Int x, Int y, Int z) {
  return cantor_pair(cantor_pair(x, y), z);
}
} // namespace util

enum token_class : unsigned char {
  IMPORT_KEYWORD,
  EXPORT_KEYWORD,
  FUN_KEYWORD,
  IDENTIFIER,
  SEMICOLON,
  COLON,
  DOUBLE_COLON,
  TOK_EOF,
  UNIMPLEMENTED,
};


auto tc2str(token_class tc) {
  switch (tc) {
    case IMPORT_KEYWORD:
      return "import";
    case EXPORT_KEYWORD:
      return "export";
    case FUN_KEYWORD:
      return "fun";
    case IDENTIFIER:
      return "identifier";
    case SEMICOLON:
      return ";";
    case COLON:
      return ":";
    case DOUBLE_COLON:
      return "::";
    case UNIMPLEMENTED:
      return "unimplemented";
  }
}


auto sources = [] {
  return vector{
    R"(
        import EXIT_SUCCESS from std;
        import fib;

        export fun main() -> int {
            const num = fib::fib(3);
            return std::EXIT_SUCCESS;
        }
    )",
    R"(
        export module fib;

        export fun fib(n: int) -> int
    )",
    R"(
        module fib;

        export fun fib(n: int) {
            if (n < 3) {
                return n;
            } else {
                return fib(n - 1) + fib(n - 2);
            }
        }
    )",
    R"(
        export module std;

        export const EXIT_SUCCESS = 0;
    )"
  };
}();


struct source_file_str_id {
  const char *source_file_str;
  int source_file_id;
};

using token_t = int;
namespace token {
auto encode_token(const token_class tc, const int seed = 0) -> token_t {
  return (tc << 24) | (seed & 0x00FF'FFFF);
}


auto decode_token(int tok) -> pair<token_class, int> {
  constexpr auto msk = 0x00FF'FFFF;
  auto cls = static_cast<token_class>((tok & ~msk) >> 24);
  auto pos = tok & msk;

  return {cls, pos};
}

} // namespace token

struct token_source_loc {
  int tok_id;
  int source_loc_id;
};


struct source_loc {
  int source_loc_id;
  int line_no;
  int col_no;
  int source_file_id;
};



struct transition_kv {
  // to implement transitions for the lexer part, we need to map from
  // token (character) to the state to jump to.
  // normally this would be done with a char array, but if we want to
  // support unicode in the future, 1 character != 1 byte, so that won't work.
  // instead, we recognise a multibyte character is of the class "character",
  // so we can reduce multibyte sequenes of chars back down to something that is
  // 1 byte in length, effectively divorcing the "kind" of character from the
  // literal text of the character.
  // for the time being, store the char type and where to jump to next
  // as a packed struct
  unsigned char key_char_type;
  unsigned char _pad0;
  unsigned short value_next_state;

  constexpr transition_kv(const unsigned char key, const ParserStateType val) {
    key_char_type = key;
    _pad0 = 0;
    value_next_state = static_cast<unsigned short>(val);
  }
};

auto get_transition_for(
  const char key,
  const vector<transition_kv> &transition_table,
  const ParserStateType default_state = P_ERR
) -> ParserStateType {
  for (const auto kv: transition_table) {
    const auto [k, _, v] = kv;
    if (k == key) { return v; }
  }

  return default_state;
}


struct cursors {
  vector<string_view> source_views;
  vector<int> indices;
  vector<vector<int>> line_starts;
};


struct transition_record {
  int source_file_id;
  ParserStateType next_parse_state;
};


auto init_cursors(const vector<vector<char>> &source_files) -> cursors {
  auto curs = cursors{};
  for (const auto &source_file: source_files) {
    curs.source_views.emplace_back(source_file.begin(), source_file.end());
    curs.indices.emplace_back(0);
    curs.line_starts.emplace_back(vector{0});
  }

  return curs;
}


#if 0
auto transition_start(
  const vector<int> &source_file_ids,
  const vector<string_view> &source_views,
  const vector<int> &indices,
  const vector<transition_kv> &transition_table,
  vector<transition_record> &destinations
) -> void {
  for (const auto source_id : source_file_ids) {
    const auto sv = source_views[source_id];
    const auto index = indices[source_id];

    if (index != sv.size()) {
      const auto lookahead = sv[index];
      auto dest = get_transition_for(lookahead, transition_table);

      destinations.emplace_back(source_id, dest);
    } else {
      destinations.emplace_back(source_id, NT_EOF);
    }
  }
}
#endif
struct ppstate {
  vector<int> source_file_ids;
};


// can change states without necessarily reading the next input
// in fact, only read next input upon a "match"
//auto naive_parse(const vector<vector<char>> &source_files) {
//  const auto n_source_files = source_files.size();
//  auto curs = init_cursors(source_files);
//
//  auto parse_states = vector<ppstate>{N_PARSE_STATES};
//  auto next_pstates = vector<vector<transition_record>>{N_PARSE_STATES};
//
//  auto sf_return_stacks = vector<vector<ParserStateType>>{n_source_files};
//  auto sf_token_streams = vector<vector<token_t>>{n_source_files};
//
//  // starting grammar symbol
//  for (auto i = 0; i < n_source_files; ++i) {
//    parse_states[NT_SOURCE_FILE_ENTER].source_file_ids.emplace_back(i);
//  }
//
//  while (parse_states[NT_EOF].source_file_ids.size() != n_source_files) {
//    [&](/* NT_SOURCE_FILE_ENTER */) {
//      // we need to read a token first.
//      // transition everyone to T_START
//      // set-up the place to return to.
//      // set-up the place to place return results.
//      for (const auto sf_ids : parse_states[NT_SOURCE_FILE_ENTER].source_file_ids) {
//        next_pstates[NT_SOURCE_FILE_ENTER].emplace_back(sf_ids, T_START);
//        sf_return_stacks[sf_ids].emplace_back(NT_SOURCE_FILE_BRANCH);
//      }
//    }();
//
//    [&](/* NT_SOURCE_FILE_BRANCH */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//        tt.emplace_back(TOK_EOF, NT_EOF);
//        return tt;
//      }();
//
//      for (const auto sf_id : parse_states[NT_SOURCE_FILE_BRANCH].source_file_ids) {
//
//      }
//    }();
//
//    [&](/* T_START */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//
//        for (auto c = 'a'; c < 'z'; ++c) {
//          auto cls = T_IDENT;
//          switch (c) {
//            default:
//            case 'c': cls = T_IDENT_OR_C;
//              break; // const, ...
//            case 'i': cls = T_IDENT_OR_I;
//              break; // import, int, ...
//            case 'e': cls = T_IDENT_OR_E;
//              break; // export, ...
//            case 'f': cls = T_IDENT_OR_F;
//              break; // fun, from, ...
//            case 'm': cls = T_IDENT_OR_M;
//              break; // module, ...
//            case 'r': cls = T_IDENT_OR_R;
//              break; // return, ...
//          }
//
//          tt.emplace_back(c, cls);
//        }
//        for (auto c = 'A'; c < 'Z'; ++c) {
//          tt.emplace_back(c, T_IDENT);
//        }
//
//        tt.emplace_back('(', T_LPAREN);
//        tt.emplace_back(')', T_RPAREN);
//        tt.emplace_back(':', T_COLON_OR_SCOPE);
//        tt.emplace_back(';', T_SEMICOLON);
//        tt.emplace_back('\n', T_LF_OR_LFCR);
//
//        // whitespace (simply thrown out)
//        for (const auto c : WS_CHARS) {
//          tt.emplace_back(' ', T_START);
//        }
//
//        // TODO: add the rest
//
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      // just a simple transition step.
//      // need to read a char
//      // potentially also update location in source file?
//      for (const auto sf_id : parse_states[T_START].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = NT_EOF;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_START].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_LF_OR_LFCR */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//
//        tt.emplace_back('\r', T_LFCR);
//
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      // just a simple transition step.
//      // need to read a char
//      // potentially also update location in source file?
//      for (const auto sf_id : parse_states[T_LF_OR_LFCR].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        ParserStateType next_state = T_LF;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, T_LF);
//        }
//
//        next_pstates[T_LF_OR_LFCR].emplace_back(sf_id, next_state);
//      }
//    }();
//
//    [&](/* T_LF */) {
//      // what is the action to take here?
//      for (const auto sf_id : parse_states[T_LF].source_file_ids) {
//        const auto index = curs.indices[sf_id];
//
//        next_pstates[T_LF].emplace_back(sf_id, T_START);
//        curs.line_starts[sf_id].emplace_back(index);
//      }
//
//    }();
//
//    [&](/* T_IDENT */) {
//      // TODO: another match
//    }();
//
//    [&](/* T_IDENT_OR_I */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//
//        tt.emplace_back('m', T_IDENT_OR_IM);
//        tt.emplace_back('n', T_IDENT_OR_IN);
//
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_I].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_I].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IDENT_OR_IM */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//
//        tt.emplace_back('p', T_IDENT_OR_IMP);
//
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_IM].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_IM].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IDENT_OR_IMP */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//        tt.emplace_back('o', T_IDENT_OR_IMPO);
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_IMP].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_IMP].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IDENT_OR_IMPO */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//        tt.emplace_back('r', T_IDENT_OR_IMPOR);
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_IMPO].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_IMPO].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IDENT_OR_IMPOR */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//        tt.emplace_back('t', T_IDENT_OR_IMPORT);
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_IMPOR].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_IMPOR].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IDENT_OR_IMPORT */) {
//      const auto transition_table = [] {
//        auto tt = vector<transition_kv>{};
//        for (const auto c : WS_CHARS) {
//          tt.emplace_back(c, T_IMPORT);
//        }
//        return tt;
//      }();
//
//      // TODO: dedup chance #1
//      for (const auto sf_id : parse_states[T_IDENT_OR_IMPORT].source_file_ids) {
//        const auto sv = curs.source_views[sf_id];
//        const auto end = sv.size();
//        const auto index = curs.indices[sf_id];
//
//        auto next_index = index;
//        ParserStateType next_state = T_IDENT;
//        if (index < end) {
//          const auto lookahead = sv[index];
//          next_state = get_transition_for(lookahead, transition_table, next_state);
//          next_index = index + 1;
//        }
//
//        next_pstates[T_IDENT_OR_IMPORT].emplace_back(sf_id, next_state);
//        curs.indices[sf_id] = next_index;
//      }
//    }();
//
//    [&](/* T_IMPORT */) {
//      // need to:
//      // store the token in the place that was set-up for us
//      // set our next state from the place that was set-up for us
//      for (const auto sf_id : parse_states[T_IMPORT].source_file_ids) {
//        auto &sf_stack = sf_return_stacks[sf_id];
//        const auto next_state = sf_stack.back();
//        sf_stack.pop_back();
//
//        // TODO: keep proper track of source location!!!!!!!
//        // and use a proper token_id.
//        sf_token_streams[sf_id].push_back(token::encode_token(IMPORT_KEYWORD));
//
//        next_pstates[T_IMPORT].emplace_back(sf_id, next_state);
//      }
//    }();
//
//    [&](/* update ATN */) {
//      for (auto pstate = 0; pstate < N_PARSE_STATES; ++pstate) {
//        parse_states[pstate].source_file_ids.clear();
//      }
//
//      for (auto pstate = 0; pstate < N_PARSE_STATES; ++pstate) {
//        for (const auto [sf_id, next_state] : next_pstates[pstate]) {
//          parse_states[next_state].source_file_ids.emplace_back(sf_id);
//        }
//        next_pstates[pstate].clear();
//      }
//    }();
//
//#ifndef NDEBUG
//    cout << "states" << endl;
//    cout << "------" << endl;
//    cout << "T_START: " << parse_states[T_START].source_file_ids << endl;
//    cout << "T_LF_OR_LFCR: " << parse_states[T_LF_OR_LFCR].source_file_ids << endl;
//    cout << "T_LF: " << parse_states[T_LF].source_file_ids << endl;
//    cout << "T_IDENT_OR_I: " << parse_states[T_IDENT_OR_I].source_file_ids << endl;
//    cout << "T_IDENT_OR_IM: " << parse_states[T_IDENT_OR_IM].source_file_ids << endl;
//    cout << "T_IDENT_OR_IMP: " << parse_states[T_IDENT_OR_IMP].source_file_ids << endl;
//    cout << "T_IDENT_OR_IMPO: " << parse_states[T_IDENT_OR_IMPO].source_file_ids << endl;
//    cout << "T_IDENT_OR_IMPOR: " << parse_states[T_IDENT_OR_IMPOR].source_file_ids << endl;
//    cout << "T_IDENT_OR_IMPORT: " << parse_states[T_IDENT_OR_IMPORT].source_file_ids << endl;
//    cout << "T_IDENT: " << parse_states[T_IDENT].source_file_ids << endl;
//    cout << "T_IMPORT: " << parse_states[T_IMPORT].source_file_ids << endl;
//    cout << endl;
//#endif
//  }
//}


enum class lex_states {
  T_START = 0,

  // -- identifiers (left-factored to remove common prefix)
  T_IDENT_OR_C,
  T_IDENT_OR_CO,
  T_IDENT_OR_CON,
  T_IDENT_OR_CONS,
  T_IDENT_OR_CONST,
  T_CONST,
  T_IDENT_OR_E,
  T_IDENT_OR_EL,
  T_IDENT_OR_ELS,
  T_IDENT_OR_ELSE,
  T_ELSE,
  T_IDENT_OR_EX,
  T_IDENT_OR_EXP,
  T_IDENT_OR_EXPO,
  T_IDENT_OR_EXPOR,
  T_IDENT_OR_EXPORT,
  T_EXPORT,
  T_IDENT_OR_F,
  T_IDENT_OR_FR,
  T_IDENT_OR_FRO,
  T_IDENT_OR_FROM,
  T_FROM,
  T_IDENT_OR_FU,
  T_IDENT_OR_FUN,
  T_FUN,
  T_IDENT_OR_I,
  T_IDENT_OR_IF,
  T_IF,
  T_IDENT_OR_IN,
  T_IDENT_OR_INT,
  T_INT,
  T_IDENT_OR_IM,
  T_IDENT_OR_IMP,
  T_IDENT_OR_IMPO,
  T_IDENT_OR_IMPOR,
  T_IDENT_OR_IMPORT,
  T_IMPORT,
  T_IDENT_OR_M,
  T_IDENT_OR_MO,
  T_IDENT_OR_MOD,
  T_IDENT_OR_MODU,
  T_IDENT_OR_MODUL,
  T_IDENT_OR_MODULE,
  T_MODULE,
  T_IDENT_OR_R,
  T_IDENT_OR_RE,
  T_IDENT_OR_RET,
  T_IDENT_OR_RETU,
  T_IDENT_OR_RETUR,
  T_IDENT_OR_RETURN,
  T_RETURN,
  T_IDENT,
  // -- end identifiers
  // -- numbers
  T_NUMBER_0,   // 0\S
  T_NUMBER_HEX, // 0x
  T_NUMBER_OCT, // 0\d
  T_NUMBER_BIN, // 0b
  T_NUMBER_DEC, // [1-9]
  // TODO fractions, exponents, etc.
  // -- end numbers
  // -- (select) operators
  T_PLUS_OR_INC_OR_ASSIGN,
  T_PLUS,
  T_INC,
  T_PLUS_ASSIGN,
  T_MINUS_OR_DEC_OR_RARROW_OR_ASSIGN,
  T_MINUS,
  T_DEC,
  T_RARROW,
  T_MINUS_ASSIGN,
  T_MUL_OR_ASSIGN,
  T_MUL,
  T_MUL_ASSIGN,
  T_DIV_OR_COMMENT_OR_ASSIGN,
  T_DIV,
  T_MULTINE_COMMENT,
  T_EOL_COMMENT,
  T_DIV_ASSIGN,
  T_LT_OR_BSHIFT_L,
  T_LT,
  T_GT,
  T_COMMA,
  T_EQ_OR_ASSIGN,
  T_EQ,
  T_ASSIGN,
  T_SEMICOLON,
  T_COLON_OR_SCOPE,
  T_COLON,
  T_SCOPE,
  T_QUESTION,
  // -- end operators
  // -- punctuation
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  // -- end punctuation
  // -- whitespace
  T_CR_OR_CRLF,
  T_LF_OR_LFCR,
  T_CR,
  T_LF,
  T_CRLF,
  T_LFCR,
  // -- end whitespace
  N_LEX_STATES,
};


// 0 .. 11 | 12 .. 31 | 32 .. 63
//   colno |  lineno  | source offset


auto naive_lex(const vector<vector<char>> &source_files) -> void {}


//TEST_CASE("some try_parse test", "[parallel]") {
int main() {
  auto v = vector<vector<char>>{};
  for (const auto &source: sources) {
    v.emplace_back();
    for (auto *p = source; *p != 0; ++p) { v.back().push_back(*p); }
  }

  naive_lex(v);
}
