# Umi (海) Language Specification

## 0. Guiding Principles

- The point of a program is to take data and instructions and perform transformations
  - Hence, "computer"
- There are only code and data -- anything that obscures that fact should be removed.
- Consistency in notation is key.
  - Prefer removing degrees of freedom for consistency
  - Exceptions to the rule should be minimal and well-justified
  - Examples of exceptions with good justifications:
    - allowing <var>.member() syntax through UCS to allow for a different "mindset" when developing code and for problems with are best expressed as a chain.
    - having a ternary operator for syntactic sugar because it is terse, well understood, and frankly cool.

## 1. Lexical Analysis

### 1.1 Character Source set

Umi source files are UTF-8 encoded text files with the *.umi* extension.

The **source character set** is as follows

[same as C++](https://en.cppreference.com/w/cpp/language/charset)

| Code Unit | Character | Glyph |
|:---------:|-----------|-------|
|  U+0009   | Tab (\t)  |       |
|  U+000B   |           |       |
|           |           |       |
|           |           |       |


## 2. Grammar

### 2.1 eBNF

```shell
Program = { ModuleStatement } { (FunctionDefinition | TopLevelStatement) };

EmptyStatement = ";";
TopLevelEntity = ImportStatement
               | ExportStatement
               | EmptyStatement
               | FunctionDefinition
               | VariableDefinition
               | ExportedDefinition;

ExportedDefinition = "export" (FunctionDefinition | VariableDefinition | Identifier);

ModuleStatement = "module", Identifier, ";";
ImportStatement = "import", Identifier, ";";
ExportStatement = "export", Identifier, ";";

StatementList = "{" { BlockStatement } "}"
BlockStatement = VariableDefinition
               | IfStatement
               | ReturnStatement;
               | EmptyStatement;

IfStatement = "if" "(" Expr ")" StatementList [ "else" StatementList ]
ReturnStatement = "return" Expr ";";

BindingSpec = ("const" | "let" | "&" | "auto");
VariableDefinition = BindingSpec, VariableIdentifier, ":", Type, "=", Expr

Identifier = UnqualifiedName
           | FullyQualifiedName
           | EscapedName
UnqualifiedName = "$_a-zA-Z", { "_a-zA-Z0-9" };
FullyQualifiedName = UnqualifiedName, "::", { Identifier };
EscapedName = "`", <any other char except backtick> ,"`"
VariableIdentifier = UnqualifiedName | EscapedName;


Type = TypeIdentifier, [TypeModifier];
TypeModifier = "const" | "restrict" | "volatile" | Array | Pointer;
TypeIdentifier = Identifier
               | PrimitiveType
PrimitiveType = "int" | "float" | "char" | "bool" | "i8" | "i16" | ...;
Array = "[", Rank, "]", { Array };
Rank = [\d, ","] # 0 or more


FunctionHeading = "fun", Identifier, "(", { Parameter, "," }, ")", ":", Type;
Parameter = BindingSpec?, VariableIdentifier, ":", Type;
FunctionDefinition = FunctionHeading FunctionBody;
FunctionBody = StatementList
             | "=" Expr ";";

MacroDefinition = "macro", Identifier, "(", ...;
macro define(lhs: symbol, rhs: symbol) {
  return `const! 'lhs = 'rhs`;
}


Expr = FunctionCallExpr
     | Identifier
     | BinaryExpr
     | Literal;

FunctionCallExpr = Identifier, "(", { Argument, "," }, "}";
Argument = Identifier
         | Expr; 

BinaryExpr = Expr, BinaryOp, Expr
BinaryOp = "+" | "-" | "<";

Literal = IntegerLiteral
        | FloatingPointLiteral;
        
IntegerLiteral = DigitSequence;
FloatingPointLiteral = IntegerLiteral, { Fractional }, { DecimalExponent };
Fractional = ".", DigitSequence;
DecimalExponent = ("e" | "E"), ("+" | "-"), DigitSequence;

A = ".", DigitSequence, { DecimalExponent };
DigitSequence 

BasePrefix = "0", ("x" | "X" | "0" | "b" | "B");
DigitSequence = ["1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"];


UnaryOperators = "'" | "." | "->" | "-" | "+" | "&" | "#";
BinOps = "@" | "+" | "=" | "==" | "," | "::"
```
    