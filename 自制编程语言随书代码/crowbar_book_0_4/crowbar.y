%{
#include <stdio.h>
#include "crowbar.h"
#define YYDEBUG 1
%}
%union {
    char                *identifier;
    CRB_ParameterList   *parameter_list;
    ArgumentList        *argument_list;
    Expression          *expression;
    ExpressionList      *expression_list;
    Statement           *statement;
    StatementList       *statement_list;
    CRB_Block           *block;
    Elsif               *elsif;
    AssignmentOperator  assignment_operator;
    IdentifierList      *identifier_list;
}
%token <expression>     INT_LITERAL
%token <expression>     DOUBLE_LITERAL
%token <expression>     STRING_LITERAL
%token <expression>     REGEXP_LITERAL
%token <identifier>     IDENTIFIER
%token FUNCTION IF ELSE ELSIF WHILE FOR FOREACH RETURN_T BREAK CONTINUE NULL_T
        LP RP LC RC LB RB SEMICOLON COLON COMMA ASSIGN_T LOGICAL_AND LOGICAL_OR
        EQ NE GT GE LT LE ADD SUB MUL DIV MOD TRUE_T FALSE_T EXCLAMATION DOT
        ADD_ASSIGN_T SUB_ASSIGN_T MUL_ASSIGN_T DIV_ASSIGN_T MOD_ASSIGN_T
        INCREMENT DECREMENT CLOSURE GLOBAL_T TRY CATCH FINALLY THROW
        FINAL
%type   <parameter_list> parameter_list
%type   <argument_list> argument_list
%type   <expression> expression expression_opt
        assignment_expression logical_and_expression logical_or_expression
        equality_expression relational_expression
        additive_expression multiplicative_expression
        unary_expression postfix_expression primary_expression array_literal
        closure_definition
%type   <expression_list> expression_list
%type   <statement> statement global_statement
        if_statement while_statement for_statement foreach_statement
        return_statement break_statement continue_statement try_statement
        throw_statement
%type   <statement_list> statement_list
%type   <block> block
%type   <elsif> elsif elsif_list
%type   <assignment_operator> assignment_operator
%type   <identifier> identifier_opt label_opt
%type   <identifier_list> identifier_list
%%
translation_unit
        : definition_or_statement
        | translation_unit definition_or_statement
        ;
definition_or_statement
        : function_definition
        | statement
        {
            CRB_Interpreter *inter = crb_get_current_interpreter();

            inter->statement_list
                = crb_chain_statement_list(inter->statement_list, $1);
        }
        ;
function_definition
        : FUNCTION IDENTIFIER LP parameter_list RP block
        {
            crb_function_define($2, $4, $6);
        }
        | FUNCTION IDENTIFIER LP RP block
        {
            crb_function_define($2, NULL, $5);
        }
        ;
parameter_list
        : IDENTIFIER
        {
            $$ = crb_create_parameter($1);
        }
        | parameter_list COMMA IDENTIFIER
        {
            $$ = crb_chain_parameter($1, $3);
        }
        ;
argument_list
        : assignment_expression
        {
            $$ = crb_create_argument_list($1);
        }
        | argument_list COMMA assignment_expression
        {
            $$ = crb_chain_argument_list($1, $3);
        }
        ;
statement_list
        : statement
        {
            $$ = crb_create_statement_list($1);
        }
        | statement_list statement
        {
            $$ = crb_chain_statement_list($1, $2);
        }
        ;
expression
        : assignment_expression
        | expression COMMA assignment_expression
        {
            $$ = crb_create_comma_expression($1, $3);
        }
        ;
assignment_expression
        : logical_or_expression
        | postfix_expression assignment_operator assignment_expression
        {
            $$ = crb_create_assign_expression(CRB_FALSE, $1, $2, $3);
        }
        | FINAL postfix_expression assignment_operator assignment_expression
        {
            $$ = crb_create_assign_expression(CRB_TRUE, $2, $3, $4);
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
            $$ = crb_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
        }
        ;
logical_and_expression
        : equality_expression
        | logical_and_expression LOGICAL_AND equality_expression
        {
            $$ = crb_create_binary_expression(LOGICAL_AND_EXPRESSION, $1, $3);
        }
        ;
equality_expression
        : relational_expression
        | equality_expression EQ relational_expression
        {
            $$ = crb_create_binary_expression(EQ_EXPRESSION, $1, $3);
        }
        | equality_expression NE relational_expression
        {
            $$ = crb_create_binary_expression(NE_EXPRESSION, $1, $3);
        }
        ;
relational_expression
        : additive_expression
        | relational_expression GT additive_expression
        {
            $$ = crb_create_binary_expression(GT_EXPRESSION, $1, $3);
        }
        | relational_expression GE additive_expression
        {
            $$ = crb_create_binary_expression(GE_EXPRESSION, $1, $3);
        }
        | relational_expression LT additive_expression
        {
            $$ = crb_create_binary_expression(LT_EXPRESSION, $1, $3);
        }
        | relational_expression LE additive_expression
        {
            $$ = crb_create_binary_expression(LE_EXPRESSION, $1, $3);
        }
        ;
additive_expression
        : multiplicative_expression
        | additive_expression ADD multiplicative_expression
        {
            $$ = crb_create_binary_expression(ADD_EXPRESSION, $1, $3);
        }
        | additive_expression SUB multiplicative_expression
        {
            $$ = crb_create_binary_expression(SUB_EXPRESSION, $1, $3);
        }
        ;
multiplicative_expression
        : unary_expression
        | multiplicative_expression MUL unary_expression
        {
            $$ = crb_create_binary_expression(MUL_EXPRESSION, $1, $3);
        }
        | multiplicative_expression DIV unary_expression
        {
            $$ = crb_create_binary_expression(DIV_EXPRESSION, $1, $3);
        }
        | multiplicative_expression MOD unary_expression
        {
            $$ = crb_create_binary_expression(MOD_EXPRESSION, $1, $3);
        }
        ;
unary_expression
        : postfix_expression
        | SUB unary_expression
        {
            $$ = crb_create_minus_expression($2);
        }
        | EXCLAMATION unary_expression
        {
            $$ = crb_create_logical_not_expression($2);
        }
        ;
postfix_expression
        : primary_expression
        | postfix_expression LB expression RB
        {
            $$ = crb_create_index_expression($1, $3);
        }
        | postfix_expression DOT IDENTIFIER
        {
            $$ = crb_create_member_expression($1, $3);
        }
        | postfix_expression LP argument_list RP
        {
            $$ = crb_create_function_call_expression($1, $3);
        }
        | postfix_expression LP RP
        {
            $$ = crb_create_function_call_expression($1, NULL);
        }
        | postfix_expression INCREMENT
        {
            $$ = crb_create_incdec_expression($1, INCREMENT_EXPRESSION);
        }
        | postfix_expression DECREMENT
        {
            $$ = crb_create_incdec_expression($1, DECREMENT_EXPRESSION);
        }
        ;
primary_expression
        : LP expression RP
        {
            $$ = $2;
        }
        | IDENTIFIER
        {
            $$ = crb_create_identifier_expression($1);
        }
        | INT_LITERAL
        | DOUBLE_LITERAL
        | STRING_LITERAL
        | REGEXP_LITERAL
        | TRUE_T
        {
            $$ = crb_create_boolean_expression(CRB_TRUE);
        }
        | FALSE_T
        {
            $$ = crb_create_boolean_expression(CRB_FALSE);
        }
        | NULL_T
        {
            $$ = crb_create_null_expression();
        }
        | array_literal
        | closure_definition
        ;
array_literal
        : LC expression_list RC
        {
            $$ = crb_create_array_expression($2);
        }
        | LC expression_list COMMA RC
        {
            $$ = crb_create_array_expression($2);
        }
        ;
closure_definition
        : CLOSURE IDENTIFIER LP parameter_list RP block
        {
            $$ = crb_create_closure_definition($2, $4, $6);
        }
        | CLOSURE IDENTIFIER LP RP block
        {
            $$ = crb_create_closure_definition($2, NULL, $5);
        }
        | CLOSURE LP parameter_list RP block
        {
            $$ = crb_create_closure_definition(NULL, $3, $5);
        }
        | CLOSURE LP RP block
        {
            $$ = crb_create_closure_definition(NULL, NULL, $4);
        }
        ;
expression_list
        : /* empty */
        {
            $$ = NULL;
        }
        | assignment_expression
        {
            $$ = crb_create_expression_list($1);
        }
        | expression_list COMMA assignment_expression
        {
            $$ = crb_chain_expression_list($1, $3);
        }
        ;
statement
        : expression SEMICOLON
        {
          $$ = crb_create_expression_statement($1);
        }
        | global_statement
        | if_statement
        | while_statement
        | for_statement
        | foreach_statement
        | return_statement
        | break_statement
        | continue_statement
        | try_statement
        | throw_statement
        ;
global_statement
        : GLOBAL_T identifier_list SEMICOLON
        {
            $$ = crb_create_global_statement($2);
        }
        ;
identifier_list
        : IDENTIFIER
        {
            $$ = crb_create_global_identifier($1);
        }
        | identifier_list COMMA IDENTIFIER
        {
            $$ = crb_chain_identifier($1, $3);
        }
        ;
if_statement
        : IF LP expression RP block
        {
            $$ = crb_create_if_statement($3, $5, NULL, NULL);
        }
        | IF LP expression RP block ELSE block
        {
            $$ = crb_create_if_statement($3, $5, NULL, $7);
        }
        | IF LP expression RP block elsif_list
        {
            $$ = crb_create_if_statement($3, $5, $6, NULL);
        }
        | IF LP expression RP block elsif_list ELSE block
        {
            $$ = crb_create_if_statement($3, $5, $6, $8);
        }
        ;
elsif_list
        : elsif
        | elsif_list elsif
        {
            $$ = crb_chain_elsif_list($1, $2);
        }
        ;
elsif
        : ELSIF LP expression RP block
        {
            $$ = crb_create_elsif($3, $5);
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
            $$ = crb_create_while_statement($1, $4, $6);
        }
        ;
for_statement
        : label_opt FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
          expression_opt RP block
        {
            $$ = crb_create_for_statement($1, $4, $6, $8, $10);
        }
        ;
foreach_statement
        : label_opt FOREACH LP IDENTIFIER COLON expression RP block
        {
            $$ = crb_create_foreach_statement($1, $4, $6, $8);
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
            $$ = crb_create_return_statement($2);
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
            $$ = crb_create_break_statement($2);
        }
        ;
continue_statement
        : CONTINUE identifier_opt SEMICOLON
        {
            $$ = crb_create_continue_statement($2);
        }
        ;
try_statement
        : TRY block CATCH LP IDENTIFIER RP block FINALLY block
        {
            $$ = crb_create_try_statement($2, $5, $7, $9);
        }
        | TRY block FINALLY block
        {
            $$ = crb_create_try_statement($2, NULL, NULL, $4);
        }
        | TRY block CATCH LP IDENTIFIER RP block
        {
            $$ = crb_create_try_statement($2, $5, $7, NULL);
        }
throw_statement
        : THROW expression SEMICOLON
        {
            $$ = crb_create_throw_statement($2);
        }
block
        : LC statement_list RC
        {
            $$ = crb_create_block($2);
        }
        | LC RC
        {
            $$ = crb_create_block(NULL);
        }
        ;
%%
