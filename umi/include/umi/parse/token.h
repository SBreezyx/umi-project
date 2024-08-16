#ifndef UMI_TOKEN_H
#define UMI_TOKEN_H


namespace umi {
using token_type = int;

enum token_class : token_type {
  // N.B.: this must be first for dynamic digit sequencing
  DIGIT_SEQUENCE = -1,

  ERROR,
  FILE_START,
  FILE_END,

  FUN_KEYWORD,
  MODULE_KEYWORD,
  EXPORT_KEYWORD,
  IMPORT_KEYWORD,
  FROM_KEYWORD,
  INT_KEYWORD,

  IF_KEYWORD,
  ELSE_KEYWORD,

  CONST_KEYWORD,
  TYPE_QUALIFIER = CONST_KEYWORD,
  LET_KEYWORD = CONST_KEYWORD,
  RETURN_KEYWORD,

  WHITESPACE,
  SINGLE_COLON,
  SINGLE_EQUALS,
  PERIOD,
  SEMICOLON,
  LEFT_PARENS,
  RIGHT_PARENS,
  LEFT_BRACE,
  RIGHT_BRACE,
  PLUS,
  MINUS,
  FWD_SLASH,
  BCK_SLASH,
  COMMA,
  QUESTION,
  LEFT_ANGLE,
  RIGHT_ANGLE,
  COMMENT_EOL,

// NB: this MUST be last as the "identifier" token_class is used
// to quickly figure out if a given token_class is reserved token_class
// or something of "the class" identifier.
  IDENTIFIER,
};

constexpr auto is_token_class(token_class cls, token_type tok) {
  if (cls == token_class::DIGIT_SEQUENCE) {
    return tok <= token_class::DIGIT_SEQUENCE;
  } else if (cls == token_class::IDENTIFIER) {
    return tok >= token_class::IDENTIFIER;
  } else {
    return tok == cls;
  }
}

constexpr auto token_class_of(token_type tok) -> token_class {
  if (is_token_class(DIGIT_SEQUENCE, tok)) {
    return DIGIT_SEQUENCE;
  } else if (is_token_class(IDENTIFIER, tok)) {
    return IDENTIFIER;
  } else {
    return static_cast<token_class>(tok);
  }
}

template<token_class Class>
auto next_token_id(token_type curr_id = Class) -> token_type {
  if constexpr (Class == token_class::DIGIT_SEQUENCE) {
    return curr_id - 1;
  } else if constexpr (Class == token_class::IDENTIFIER) {
    return curr_id + 1;
  } else {
    return token_class::ERROR;
  }
}

#ifndef NDEBUG
constexpr auto tokenc2str(token_type tok) {
  const auto tok_class = token_class_of(tok);
  switch (tok_class) {
    case IDENTIFIER: return "ident";
    case DIGIT_SEQUENCE: return "digits";
    case FUN_KEYWORD: return "fun";
    case SEMICOLON: return ";";
    case LEFT_PARENS: return "(";
    case MODULE_KEYWORD: return "module";
    case IMPORT_KEYWORD: return "import";
    case EXPORT_KEYWORD: return "export";
    case LEFT_ANGLE: return "<";
    case ERROR: return "ERROR";
    case FILE_END: return "eof";
    case FROM_KEYWORD: return "from";
    case INT_KEYWORD: return "int";
    case CONST_KEYWORD: return "const";
    case RETURN_KEYWORD: return "return";
    case WHITESPACE: return "WHITESPACE";
    case SINGLE_COLON: return ":";
    case SINGLE_EQUALS: return "=";
    case PERIOD: return ".";
    case RIGHT_PARENS: return ")";
    case LEFT_BRACE: return "{";
    case RIGHT_BRACE: return "}";
    case PLUS: return "+";
    case MINUS: return "-";
    case FWD_SLASH: return "/";
    case BCK_SLASH: return "\\";
    case COMMA: return ",";
    case QUESTION: return "?";
    case RIGHT_ANGLE: return ">";
    case COMMENT_EOL: return "//";
    case IF_KEYWORD: return "if";
    case ELSE_KEYWORD: return "else";
    default:
      throw "did not implement all token class string cases";
  }

}

#endif

}
#endif // UMI_TOKEN_H
