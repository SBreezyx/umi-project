grammar umi;

/*
 * first-set:
 * "module" -> module_decl
 * "export" -> module_decl, export_stmt
 * "import" -> import_stmt
 * "fun" -> fun_decl, fun_def
*/
source_file: declarations*
           | EOF;

declarations: first_declaration non_first_declaration*;

type: (fundamental_type | struct_type | union_type | user_type) type_modifier*
    | fun_type
    | '(' type ')'
    ;

fundamental_type: 'int' /* alias for int32 */ | 'uint' /* alias for uint32 */
                | 'int8' | 'int16' | 'int32' | 'int64'
                | 'uint8' | 'uint16' | 'uint32' | 'uint64'
                | 'float' /* alias for float32 */ | 'double' /* alias for float64 */
                | 'float16' | 'float32' | 'float64'
                | 'byte' | 'bool'
                | 'char' // alias for char8
                | 'char8' | 'char16' | 'char32'
                | 'void' | 'never' | 'symbol'
                ;

user_type: ident;
struct_type: 'struct' '{' member_def* '}';
union_type: 'union' '{' member_def* '}';
fun_type: '(' (type (',' type)* ','?)? ')' '->' type;

type_modifier: 'const'
             | 'volatile'
             | 'unique'
             | '[' integral_literal ']'
             | '*'
             ;

lambda_capture: ;
lambda: '[' lambda_capture ']' ('(' parameter* ')')? ('throws' type)? ('->' type)? '{' stmt '}' ;

designated_ref: decimal_literal | unqualified_id;
designated_initializer: '.' designated_ref '=' ;
initializer_list: '{' (designated_initializer? expr (',' designated_initializer? expr)* ','? )? '}' ;

upper_step: ':' (expr | 'default');
upper_slice: ':' (expr | 'default') upper_step?;
subscript_expr: expr upper_slice? ;
subscript_op: '[' subscript_expr (',' subscript_expr)* ','? ']';

//stmt_expr: '{' stmt* expr '}';
//if_expr: 'if' '(' if_init_stmt? if_cond ')' (expr | stmt_expr) 'else' (expr | stmt_expr) ('finally' compound_stmt)?;
//try_expr: 'try' (expr | stmt_expr) ('catch' (expr | stmt_expr))? ('finally' compound_stmt)? ;
//switch_expr: 'switch' ('(' switch_init_stmt switch_cond ')' )? '{' switch_label '}';
//for_expr: 'a';
//while_expr: 'b';
//do_expr: 'c';


expr: ternary_expr (',' expr)* ;

ternary_expr: binary_expr
            | 'throw' expr
            ;

binary_expr: prefix_unary_expr bin_op?;

bin_op: ('*'|'/'|'%'|'<<'|'>>'|'&'|'&^') binary_expr
      | ('+'|'-'|'^'|'|') binary_expr
      | ('=='|'!='|'<'|'<='|'>'|'>=') binary_expr
      | '&&' binary_expr
      | '||' binary_expr
      | (assign expr | '?' expr? ':' expr)
      ;

prefix_unary_expr: postfix_unary_expr
           | ('++'|'--') prefix_unary_expr
           | ('-'|'+') prefix_unary_expr
           | ('!'|'~') prefix_unary_expr
           | ('*'|'&') prefix_unary_expr
           ;

postfix_unary_expr: primary_expr postfix_unary_op* ;
postfix_unary_op: call_op
           | 'as' type
           | subscript_op
           | ('.'|'->') unqualified_id
           | ('++'|'--')
           ;

pos_arg: expr;
named_arg: unqualified_id '=' expr;
positional_args: pos_arg (',' pos_arg)+ ','? ;
named_args: named_arg (',' named_arg)+ ','? ;
argument_list: positional_args
             | named_args
             | positional_args? named_args;
call_op: '(' argument_list? ')' ;

primary_expr: ident
             | literal
             | lambda
             | type? initializer_list
             | '(' expr ')'
//             | stmt_expr
//             | if_expr
//             | switch_expr
//             | try_expr
//             | with_expr
//             | for_expr
//             | while_expr
//             | do_expr
             ;

digit: '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9';
decimal_literal: digit digit*;
decimal: '.' decimal_literal;
dec_exponent: 'e' | 'E';
plus_sign: '+';
minus_sign: '-';
mag_sign: plus_sign | minus_sign;
decimal_exponent: dec_exponent mag_sign decimal_literal;

hexit: digit | 'A' | 'a' | 'B' | 'b' | 'C' | 'c' | 'D' | 'd' | 'E' | 'e' | 'F' | 'f';
heximal: '.' hexit+;
hex_exponent: ('p' | 'P') mag_sign hexit+;
hex_prefix: '0' ('x' | 'X');
hex_literal: hex_prefix hexit+;

binary_literal: '0[bB][01]+';

octal_literal: '0[0-7]+';

integral_literal: decimal_literal | binary_literal | octal_literal | hex_literal;

literal: 'nullptr'
       | decimal_literal decimal? decimal_exponent?
       | hex_literal heximal? hex_exponent?
       | '".*?"'
       | 'true' | 'false'
       ;

assign: '='
      | '+='
      | '-='
      | '*='
      | '/='
      | '%='
      | '>>='
      | '<<='
      | '&='
      | '|='
      | '&^='
      | '^='
      ;

/*
 * first set:
 * "export"
 * "module"
 */
module_ident: ident;
module_decl: MODULE module_ident ';';

first_declaration: module_decl
         | import_decl
         | alias_decl
         | template_global_var_decl
         | fun_decl
         | struct_decl
         | enum_decl
         | union_decl
         | export_decl
         ;

non_first_declaration: import_decl
         | alias_decl
         | template_global_var_decl
         | fun_decl
         | struct_decl
         | enum_decl
         | union_decl
         | non_module_export_decl
         ;

empty_stmt: ';';

import_all: '*';
import_alias: 'as' unqualified_id;
import_one: unqualified_id import_alias? ;
import_many: import_one (',' import_one)* ',';
import_select: (import_all | import_many) 'from' ident;
import_simple: unqualified_id;
import_targets: import_simple | import_select ;
import_decl: 'import' import_targets ';' ;

/*
 * first set
 * "fun"
 *
*/
export_alias: 'as' unqualified_id;
export_all: '*';
export_one: ident export_alias?;
export_many: export_one (',' export_one)* ',';
export_names: (export_many | export_all) ';';
export_target: module_decl
             | fun_decl
             | global_var_decl
             | struct_decl
             | union_decl
             | export_names
//             | (macro_decl | macro_def)
             ;

non_module_export_target: fun_decl
             | global_var_decl
             | struct_decl
             | union_decl
             | export_names
//             | (macro_decl | macro_def)
             ;

export_decl: 'export' export_target;
non_module_export_decl: 'export' non_module_export_target;

storage_class: 'global' | 'local';
binding_type: 'auto' | 'auto!' | 'const' | 'const!' | 'let' | 'let!' | '&';
global_var_decl: storage_class? binding_type unqualified_id ':' type ';' ;
global_var_def: storage_class? binding_type unqualified_id ':' type '=' expr ';' ;

template_global_var_decl: storage_class? binding_type unqualified_id '<' unqualified_id (':' ident) '>' ';' ;
template_global_var_def: storage_class? binding_type unqualified_id '<' unqualified_id (':' ident) '>' '=' expr ';' ;

alias_decl: 'typedef' unqualified_id '=' type ';';

fun_modifier: 'tail' | 'inline';
parameter_list: (binding_type? parameter ','?)+;

fun_return_type: '->' type;
fun_ident: ident;
fun_prototype: fun_modifier? FUN fun_ident '(' parameter_list? ')' ('throws' type)?;
fun_body: compound_stmt ('finally' compound_stmt)?
        | '=' expr ';'
        | ';'
        ;

fun_decl: fun_prototype fun_return_type? fun_body;

param_name: unqualified_id;
param_names: param_name
           | '[' param_name (',' param_name)* ',' ']';

variadic_type: '...' type;
param_type: variadic_type
          | type;
parameter: (binding_type? param_names ':')? param_type;


compound_stmt: '{' stmt* '}';
continue_stmt: 'continue' ';' ;
break_stmt: 'break' ';' ;
stmt: compound_stmt
    | import_decl
    | alias_decl
    | storage_class? var_def
    | return_stmt
    | if_stmt
    | switch_stmt
    | try_stmt
    | for_stmt
    | while_stmt
    | do_stmt
    | empty_stmt
    | expr_stmt
    ;

var_def: binding_type unqualified_id (':' type)? '=' expr (','unqualified_id (':' type)? '=' expr)* ','?  ';'
       | binding_type '[' (binding_type? unqualified_id) (',' (binding_type? unqualified_id))* ','? ']' '=' expr ';'
       ;

expr_stmt: expr ';';

return_stmt: 'return' expr ';' ;

if_init_stmt: var_def;
if_cond: expr;
if_stmt: 'if' '(' if_init_stmt? if_cond ')' compound_stmt ('else' compound_stmt)? ('finally' compound_stmt)? ;

switch_init_stmt: var_def;
switch_cond: expr;
switch_case_stmt: stmt | continue_stmt;
switch_label: 'case' expr '->' switch_case_stmt
            | 'default' '->' switch_case_stmt
            ;
switch_stmt: 'switch' '(' switch_init_stmt? switch_cond ')' '{' switch_label* '}' ('finally' compound_stmt)?
           | 'switch' '{' switch_label* '}' ('finally' compound_stmt)?
           ;

catch_init_stmt: (unqualified_id ':')? type;
try_stmt: 'try' compound_stmt ('catch' '(' catch_init_stmt ')' compound_stmt)? ('finally' compound_stmt)? ;

loop_compound_statement: '{' (stmt | continue_stmt | break_stmt) '}';

for_init_stmt: var_def | empty_stmt;
for_condition_stmt: expr_stmt;
for_step_stmt: expr | empty_stmt ;
for_stmt: 'for' '(' for_init_stmt for_condition_stmt for_step_stmt ')' loop_compound_statement ('finally' compound_stmt)?
        | 'for' '(' var_def ':' expr ')' loop_compound_statement ('finally' compound_stmt)?
        ;

while_init_stmt: var_def;
while_cond: expr;
while_stmt: 'while' '(' while_init_stmt? while_cond ')' loop_compound_statement ('finally' compound_stmt)? ;

do_stmt: 'do' loop_compound_statement ('finally' compound_stmt)? 'while' '(' while_cond ')' ';';

/*
 * first set:
 * "IDENTIFIER"
 */
unqualified_id: '[_a-zA-Z][_a-zA-Z0-9]*';
ident: unqualified_id ('::' unqualified_id)*;

member_def: (unqualified_id ':')? type ';'
          | '...'ident ';' ; // should be an aggregate type

struct_decl: 'struct' ident  ';';
struct_def: 'struct' ident '{' member_def* '}' ;

union_decl: 'union' ident ';' ;
union_def: 'union' ident '{' member_def* '}' ;

enum_decl: 'enum' ident ';' ;
enum_def: 'enum' ident '{' (unqualified_id ('=' expr) ',')? '}';

//macro_decl: MACRO id '(' ((unqualified_id ':')? 'symbol' ) ')' ';' ;
//macro_def: MACRO id '(' ((unqualified_id ':')? 'symbol' ) ')' '{'  '}' ;

//const_if_init_stmt: const_var_def;
//const_if_cond: expr;
//const_if_stmt: IF_COMP '(' const_if_init_stmt? const_if_cond ')' stmt ((ELSE | ELSE_COMP) stmt)? ;

//const_try_stmt: TRY_COMP stmt ((CATCH | CATCH_COMP) ('(' catch_init_stmt ')')? stmt)? ;

// todo
//const_for_stmt: FOR_COMP '(' ')' ';';
//const_while_stmt: WHILE_COMP '(' ')' ';';
//const_do_stmt: DO_COMP stmt (WHILE | WHILE_COMP) '(' ')' ';' ;