%{
#include <stdio.h>
#include "diksamc.h"
#define YYDEBUG 1
%}
%union {
    char                *identifier;
    ParameterList       *parameter_list;
    ArgumentList        *argument_list;
    Expression          *expression;
    Statement           *statement;
    StatementList       *statement_list;
    Block               *block;
    Elsif               *elsif;
    AssignmentOperator  assignment_operator;
    DVM_BasicType       type_specifier;
}
%token <expression>     INT_LITERAL
%token <expression>     DOUBLE_LITERAL
%token <expression>     STRING_LITERAL
%token <expression>     REGEXP_LITERAL
%token <identifier>     IDENTIFIER
%token IF ELSE ELSIF WHILE FOR FOREACH RETURN_T BREAK CONTINUE
        LP RP LC RC SEMICOLON COLON COMMA ASSIGN_T LOGICAL_AND LOGICAL_OR
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD TRUE_T FALSE_T EXCLAMATION DOT
        ADD_ASSIGN_T SUB_ASSIGN_T MUL_ASSIGN_T DIV_ASSIGN_T MOD_ASSIGN_T
        INCREMENT DECREMENT TRY CATCH FINALLY THROW
        BOOLEAN_T INT_T DOUBLE_T STRING_T
%type   <parameter_list> parameter_list
%type   <argument_list> argument_list
%type   <expression> expression expression_opt
        assignment_expression logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression postfix_expression primary_expression
%type   <statement> statement
        if_statement while_statement for_statement foreach_statement
        return_statement break_statement continue_statement try_statement
        throw_statement declaration_statement
%type   <statement_list> statement_list
%type   <block> block
%type   <elsif> elsif elsif_list
%type   <assignment_operator> assignment_operator
%type   <identifier> identifier_opt label_opt
%type   <type_specifier> type_specifier
%%
translation_unit
        : definition_or_statement
        | translation_unit definition_or_statement
        ;
definition_or_statement
        : function_definition
        | statement
        {
            DKC_Compiler *compiler = dkc_get_current_compiler();

            compiler->statement_list
                = dkc_chain_statement_list(compiler->statement_list, $1);
        }
        ;
type_specifier
        : BOOLEAN_T
        {
            $$ = DVM_BOOLEAN_TYPE;
        }
        | INT_T
        {
            $$ = DVM_INT_TYPE;
        }
        | DOUBLE_T
        {
            $$ = DVM_DOUBLE_TYPE;
        }
        | STRING_T
        {
            $$ = DVM_STRING_TYPE;
        }
        ;
function_definition
        : type_specifier IDENTIFIER LP parameter_list RP block
        {
            dkc_function_define($1, $2, $4, $6);
        }
        | type_specifier IDENTIFIER LP RP block
        {
            dkc_function_define($1, $2, NULL, $5);
        }
        | type_specifier IDENTIFIER LP parameter_list RP SEMICOLON
        {
            dkc_function_define($1, $2, $4, NULL);
        }
        | type_specifier IDENTIFIER LP RP SEMICOLON
        {
            dkc_function_define($1, $2, NULL, NULL);
        }
        ;
parameter_list
        : type_specifier IDENTIFIER
        {
            $$ = dkc_create_parameter($1, $2);
        }
        | parameter_list COMMA type_specifier IDENTIFIER
        {
            $$ = dkc_chain_parameter($1, $3, $4);
        }
        ;
argument_list
        : assignment_expression
        {
            $$ = dkc_create_argument_list($1);
        }
        | argument_list COMMA assignment_expression
        {
            $$ = dkc_chain_argument_list($1, $3);
        }
        ;
statement_list
        : statement
        {
            $$ = dkc_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = dkc_chain_statement_list($1, $2);
        }
        ;
expression
        : assignment_expression
        | expression COMMA assignment_expression
        {
            $$ = dkc_create_comma_expression($1, $3);
        }
        ;
assignment_expression
        : logical_or_expression
        | postfix_expression assignment_operator assignment_expression
        {
            $$ = dkc_create_assign_expression($1, $2, $3);
        }
        ;
assignment_operator
        : ASSIGN_T
        {
            $$ = NORMAL_ASSIGN;
        }
        | ADD_ASSIGN_T
        {
            $$ = ADD_ASSIGN;
        }
        | SUB_ASSIGN_T
        {
            $$ = SUB_ASSIGN;
        }
        | MUL_ASSIGN_T
        {
            $$ = MUL_ASSIGN;
        }
        | DIV_ASSIGN_T
        {
            $$ = DIV_ASSIGN;
        }
        | MOD_ASSIGN_T
        {
            $$ = MOD_ASSIGN;
        }
        ;
logical_or_expression
        : logical_and_expression
        | logical_or_expression LOGICAL_OR logical_and_expression
        {
            $$ = dkc_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;
logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = dkc_create_binary_expression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;
equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = dkc_create_binary_expression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = dkc_create_binary_expression(NE_EXPRESSION, $1, $3);
        }
        ;
relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = dkc_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = dkc_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = dkc_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = dkc_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = dkc_create_binary_expression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = dkc_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;
multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = dkc_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = dkc_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = dkc_create_binary_expression(MOD_EXPRESSION, $1, $3);
        }
        ;
unary_expression
        : postfix_expression
        | SUB unary_expression
        {
            $$ = dkc_create_minus_expression($2);
        }
        | EXCLAMATION unary_expression
        {
            $$ = dkc_create_logical_not_expression($2);
        }
        ;
postfix_expression
        : primary_expression
        | postfix_expression LP argument_list RP
        {
            $$ = dkc_create_function_call_expression($1, $3);
        }
        | postfix_expression LP RP
        {
            $$ = dkc_create_function_call_expression($1, NULL);
        }
        | postfix_expression INCREMENT
        {
            $$ = dkc_create_incdec_expression($1, INCREMENT_EXPRESSION);
        }
        | postfix_expression DECREMENT
        {
            $$ = dkc_create_incdec_expression($1, DECREMENT_EXPRESSION);
        }
        ;
primary_expression
        : LP expression RP
        {
            $$ = $2;
        }
        | IDENTIFIER
        {
            $$ = dkc_create_identifier_expression($1);
        }
        | INT_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | REGEXP_LITERAL
        | TRUE_T
        {
            $$ = dkc_create_boolean_expression(DVM_TRUE);
        }
        | FALSE_T
        {
            $$ = dkc_create_boolean_expression(DVM_FALSE);
        }
        ;
statement
        : expression SEMICOLON
        {
          $$ = dkc_create_expression_statement($1);
        }
        | if_statement
        | while_statement
        | for_statement
        | foreach_statement
        | return_statement
        | break_statement
        | continue_statement
        | try_statement
        | throw_statement
        | declaration_statement
        ;
if_statement
        : IF LP expression RP block
        {
            $$ = dkc_create_if_statement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = dkc_create_if_statement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elsif_list
        {
            $$ = dkc_create_if_statement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elsif_list ELSE block
        {
            $$ = dkc_create_if_statement($3, $5, $6, $8);
        }
        ;
elsif_list
        : elsif
        | elsif_list elsif
        {
            $$ = dkc_chain_elsif_list($1, $2);
        }
        ;
elsif
        : ELSIF LP expression RP block
        {
            $$ = dkc_create_elsif($3, $5);
        }
        ;
label_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER COLON
        {
            $$ = $1;
        }
        ;
while_statement
        : label_opt WHILE LP expression RP block
        {
            $$ = dkc_create_while_statement($1, $4, $6);
        }
        ;
for_statement
        : label_opt FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = dkc_create_for_statement($1, $4, $6, $8, $10);
        }
        ;
foreach_statement
        : label_opt FOREACH LP IDENTIFIER COLON expression RP block
        {
            $$ = dkc_create_foreach_statement($1, $4, $6, $8);
        }
        ;
expression_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | expression
        ;
return_statement
        : RETURN_T expression_opt SEMICOLON
        {
            $$ = dkc_create_return_statement($2);
        }
        ;
identifier_opt
        : /* empty */
        {
            $$ = NULL;
        }
        | IDENTIFIER
        ;
break_statement 
        : BREAK identifier_opt SEMICOLON
        {
            $$ = dkc_create_break_statement($2);
        }
        ;
continue_statement
        : CONTINUE identifier_opt SEMICOLON
        {
            $$ = dkc_create_continue_statement($2);
        }
        ;
try_statement
        : TRY block CATCH LP IDENTIFIER RP block FINALLY block
        {
            $$ = dkc_create_try_statement($2, $5, $7, $9);
        }
        | TRY block FINALLY block
        {
            $$ = dkc_create_try_statement($2, NULL, NULL, $4);
        }
        | TRY block CATCH LP IDENTIFIER RP block
        {
            $$ = dkc_create_try_statement($2, $5, $7, NULL);
        }
throw_statement
        : THROW expression SEMICOLON
        {
            $$ = dkc_create_throw_statement($2);
        }
declaration_statement
        : type_specifier IDENTIFIER SEMICOLON
        {
            $$ = dkc_create_declaration_statement($1, $2, NULL);
        }
        | type_specifier IDENTIFIER ASSIGN_T expression SEMICOLON
        {
            $$ = dkc_create_declaration_statement($1, $2, $4);
        }
        ;
block
        : LC
        {
            $<block>$ = dkc_open_block();
        }
          statement_list RC
        {
            $<block>$ = dkc_close_block($<block>2, $3);
        }
        | LC RC
        {
            Block *empty_block = dkc_open_block();
            $<block>$ = dkc_close_block(empty_block, NULL);
        }
        ;
%%
