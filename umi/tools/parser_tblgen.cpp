#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <stack>
#include <vector>

#define skip_space(curs) for (; isspace(*(curs)); ++(curs))
#define die(x) do { std::cout << x << std::endl; exit(1); } while(false)

using namespace std;

constexpr auto EPS = 0;

struct tok {
  enum cls {
    grammar_key,
    non_term,
    term,
    re,
    semicolon,
    colon,
    lparen,
    rparen,
    alt,
    opt,
    zero_more,
    one_more,
  } kind;

  string data;
};

struct grammar {
  // a map of ID to terminal/non-terminal IDs
  map<string, int> symbols;

  vector<int> terminals;
  vector<int> non_terminals;

  // a list of productions of the form:
  // E -> T E int
  // E ->
  // where E & T are non-terminals, 'int' is a terminal
  // and E -> '' is an epsilon rule
  vector<vector<int>> productions;
};

static constexpr const char *tok_strs[] = {
  "grammar_key", "non_term", "term", "literal", "sc",
  "c", "lp", "rp", "alt", "opt", "zm", "om", "re"
};

auto operator<<(ostream &os, const tok &t) -> ostream & {
  os << tok_strs[t.kind] << ": '" << t.data << "'";

  return os;
}

auto operator<<(ostream &os, const vector<tok> &v) -> ostream & {
  if (v.empty()) {
    os << "[]";
  } else {
    auto it = v.begin();
    os << "[\n\t" << *it++;

    for (const auto end = v.end(); it != end; ++it) {
      os << "\n\t, " << *it;
    }

    os << "\n]";
  }

  return os;
}

static
auto tokenize(const string &s) -> vector<tok> {
  const auto punc = map<char, tok>{
    {';', {tok::semicolon, ";"}},
    {':', {tok::colon,     ":"}},
    {'(', {tok::lparen,    "("}},
    {')', {tok::rparen,    ")"}},
    {'|', {tok::alt,       "|"}},
    {'?', {tok::opt,       "?"}},
    {'*', {tok::zero_more, "*"}},
    {'+', {tok::one_more,  "+"}},
  };

  auto tokens = vector<tok>{};
  const auto end = s.end();
  auto in_parser_rules = true;
  auto curs = s.begin();
  while (curs != end && in_parser_rules) {
    skip_space(curs);
    if (isalpha(*curs)) {
      auto kind = islower(*curs) ? tok::non_term : tok::term;
      auto id = string{*curs++};
      for (; curs != end && isalpha(*curs) || *curs == '_'; ++curs) {
        id += *curs;
      }

      if (id == "grammar") {
        tokens.push_back({tok::grammar_key, id});
      } else {
        tokens.push_back({kind, id});
      }

      skip_space(curs);
      in_parser_rules = kind == tok::non_term || (curs != end && *curs != ':');
    } else if (*curs == '\'') {
      ++curs;
      auto term = string{};
      while (curs != end && *curs != '\'') {
        term += *curs++;
      }

      if (curs != end) {
        ++curs;
      }

      tokens.push_back({tok::re, term});
    } else if (auto f = punc.find(*curs); f != punc.end()) {
      tokens.push_back(f->second);
      ++curs;
    }
  }

  if (!in_parser_rules) {
    for (; curs != end; ++curs) {
      skip_space(curs);

      if (*curs == ':') {
        tokens.push_back({tok::colon, ":"});
      } else if (*curs == ';') {
        tokens.push_back({tok::semicolon, ";"});
      } else if (isalpha(*curs)) {
        auto term_ident = string{*curs++};
        for (; curs != end && isalpha(*curs); ++curs) {
          term_ident += *curs;
        }

        tokens.push_back({tok::term, term_ident});
      } else {
        auto re = string{*curs++};
        for (; curs != end && *curs != ';'; ++curs) {
          re += *curs;
        }

        tokens.push_back({tok::re, re});
      }
    }
  }

  return tokens;
}

static
auto accepted(tok::cls exp, vector<tok>::const_iterator it) -> bool {
  if (exp != it->kind) {
    return false;
  }

  return true;
}

static
auto accept(vector<tok>::const_iterator &it) -> tok {
  return *it++;
}

static
auto accept_if(tok::cls exp, vector<tok>::const_iterator &it) -> tok {
  if (accepted(exp, it)) {
    return accept(it);
  } else {
    die(tok_strs[exp] << "!=" << tok_strs[it->kind]);
//    return ;
  }
}

static
auto try_add_nonterm(grammar &g, set<string> &unresolved, int &id, tok nt) -> int {
  auto extant = g.symbols.find(nt.data);
  if (extant != g.symbols.end()) {
    unresolved.erase(nt.data);
    return extant->second;
  } else {
    auto new_id= ++id;
    g.non_terminals.push_back(new_id);
    g.symbols[nt.data] = new_id;
    unresolved.insert(nt.data);
    return new_id;
  }
}

static
auto add_term(grammar &g, int &id, tok t) -> void {
  g.terminals.push_back(id);
  g.symbols[t.data] = ++id;
}

static
auto gen_anonymous(tok t, int nest_level, int &id) -> pair<tok, int> {
  return std::pair{
    tok{t.kind, std::format("%s::$%d", t.data, nest_level)},
    ++id
  };
}

static
auto parse(const std::vector<tok> &tokens) -> grammar {
  auto g = grammar{};
  auto unresolved_symbols = set<string>{};
  auto id = EPS + 1;

  auto cursor = tokens.begin();
  const auto end = tokens.end();

  accept_if(tok::grammar_key, cursor);
  accept_if(tok::non_term, cursor);
  accept_if(tok::semicolon, cursor);

  while (cursor != end) {
    auto derivation_name = accept_if(tok::non_term, cursor);
    auto derv_id = try_add_nonterm(g, unresolved_symbols, id, derivation_name);
    auto derived_rules = vector<vector<int>>{{derv_id}};

    accept_if(tok::colon, cursor);

    auto nest_level = 0;
    auto derv_stack = stack<tuple<tok, int, int>>{};

    while (cursor != end && cursor->kind != tok::semicolon) {
      if (cursor->kind == tok::non_term) {
        auto part_id = try_add_nonterm(g, unresolved_symbols, id, accept(cursor));

      } else if (cursor->kind == tok::re || cursor->kind == tok::term) {
        // parse_re();
        add_term(g, id, *cursor);
      } else if (cursor->kind == tok::lparen) {
        accept(cursor);
        derv_stack.emplace(derivation_name, derv_id, nest_level++);

        tie(derivation_name, derv_id) = gen_anonymous(derivation_name, nest_level, id);

      } else if (cursor->kind == tok::rparen) {
        if (!derv_stack.empty() && get<2>(derv_stack.top()) == nest_level - 1) {
          accept(cursor);

          const auto top = derv_stack.top();
          derv_stack.pop();
          derivation_name = get<0>(top);
          derv_id = get<1>(top);
          nest_level = get<2>(top);
        } else {
          die("unmatched parentheses at nest level: " << nest_level);
        }
      } else if (cursor->kind == tok::alt) {

      } else if (cursor->kind == tok::opt) {

      } else if (cursor->kind == tok::one_more) {

      } else if (cursor->kind == tok::zero_more) {

      } else {
        die("syntax error: did not expect" << tok_strs[cursor->kind]);
      }
    }
  }

  while (cursor != end) {
    const auto term_def = accept_if(tok::term, cursor);

    accept_if(tok::semicolon, cursor);

    const auto re = accept_if(tok::re, cursor);

    unresolved_symbols.erase(term_def.data);
  }

  if (!unresolved_symbols.empty()) {
    die("not all symbols were properly defined");
  }

  return std::move(g);
}

auto main(int argc, char **argv) -> int {
  const auto grammar_spec = [&] {
    if (argc == 1) {
      auto s = string{};
      while (cin >> s);
      return s;
    } else {
      auto file = ifstream{argv[1]} >> noskipws;
      return string{istream_iterator<char>{file}, istream_iterator<char>{}};
    }
  }();

  auto toks = tokenize(grammar_spec);

  cout << toks << endl;

  return 0;
}