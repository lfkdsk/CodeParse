%{

#include <stdio.h>
#include "hobby.h"
#define YYDEBUG 1

%}

%union {
	char 			*identifier;
	ParameterList 	*parameterList;
	ArgumentList	*argumentList;
	Expression		*expression;
	Statement		*statement;
	StatementList	*statement_list;
	Block			*block;
	ElseIf 			*elseif;
	IdentifierList	*identifier_list;
}

%token <expression> INT_LITERAL
%token <expression> DOUBLE_LITERAL
%token <expression> STRING_LITERAL
%token <identifier> IDENTIFIER
%token FUNCTION IF ELSE ELSEIF WHILE FOR RETURN_T BREAK CONTINUE NULL_T
	LP RP LC RC SEMICOLON COMMA ASSIGN LOGICAL_AND LOGICAL_OR
	RQ NR GT GE LT LR ADD SUB MUL DIV MOD TRUE_T FALSE_T GLOBAL_T

%type <parameter_list> parameter_list
%type <argument_list> argumentList
%type <expression> expression expression_opt
			logical_and_expression logical_or_expression
			equiality_expression relational_expression
			additive_expression multiplicative_expression
			unary_expression primary_expression

%type <statement> statement global_statement
			if_statement while_statement for_statement
			return_statement break_statement continue_statement

%type <statement_list> statement_list

%type <block> block

%type <elseif> elseif elseif_list
%type <identifier_list> identifier_list

%%

translation_unit: definitaion_or_statement
			| translation_unit definitaion_or_statement
			;

definitaion_or_statement:
			function_definition
			| statement {
					HBB_InterPreter *inter = hbb_get_current_interpreter();

					inter->statement_list = hbb_chain_statement_list(inter->statement_list, $1);
			}
			;

			/* function funName (parameter_list) {} / not parameter */
function_definition:
			FUNCTION IDENTIFIER LP parameter_list RP block {
					hbb_function_define($2 ,$4, $6);
			}
			| FUNCTION IDENTIFIER LP RP block {
					hbb_function_define($2 , NULL, $5);
			}
			;

			/* id / list, id */
parameter_list:
			IDENTIFIER {
					$$ = hbb_create_parameter($1);
			}
			| parameter_list COMMA IDENTIFIER {
					$$ = hbb_chain_parameter($1, $3);
			}
			;

			/* ex / list , ex */
argument_list:
			expression {
					$$ = hbb_create_argument_list($1);
			}
			| argument_list COMMA expression {
					$$ = hbb_chain_argument_list($1, $3);
			}
			;

			/* st / list */
statement_list:
			statement {
					$$ = hbb_create_statement_list($1);
			}
			| statement_list statement {
					$$ = hbb_chain_statement_list($1, $2);
			}
			;

			/* id = exp / log_or */
expression:
			logical_or_expression
			| IDENTIFIER ASSIGN expression {
					$$ = hbb_create_assign_expression($1, $3);
			}
			;

			/* log_or -> and / log_or || and */
logical_or_expression:
			logical_and_expression
			| logical_or_expression LOGICAL_OR logical_and_expression {
					$$ = hbb_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
			}
			;

			/* and->equ / log_and && equ  */
logical_and_expression:
			equiality_expression
			| logical_and_expression LOGICAL_AND equiality_expression {
					$$ = hbb_create_binary_expression(LOGICAL_AND_EXPRESSION, $1 , $3);
			}
			;

			/* equ->rel / equ  */
equiality_expression:
			relational_expression
			| equiality_expression GT relational_expression {
					$$ = hbb_create_binary_expression(EQ_EXPRESSION, $1, $3);
			}
			| equiality_expression NE relational_expression {
					$$ = hbb_create_binary_expression(NE_EXPRESSION, $1, $3);
			}
			;

relational_expression:
			additive_expression
			| relational_expression GT additive_expression {
					$$ = hbb_create_binary_expression(GT_EXPRESSION, $1, $3);
			}
			| relational_expression GE additive_expression {
					$$ = hbb_create_binary_expression(GE_EXPRESSION, $1, $3);
			}
			| relational_expression LT additive_expression {
					$$ = hbb_create_binary_expression(LT_EXPRESSION, $1, $3);
			}
			| relational_expression LE additive_expression {
				 	$$ = hbb_create_binary_expression(LE_EXPRESSION, $1, $3);
			}
			;

additive_expression:
			multiplicative_expression
			| additive_expression ADD multiplicative_expression	{
					$$ = hbb_create_binary_expression(ADD_EXPRESSION, $1, $3);
			}
			| additive_expression SUB multiplicative_expression {
					$$ = hbb_create_binary_expression(SUB_EXPRESSION, $1, $3);
			}
			;

multiplicative_expression:
			unary_expression
			| multiplicative_expression MUL	unary_expression {
					$$ = hbb_create_binary_expression(MUL_EXPRESSION, $1 ,$3);
			}
			| multiplicative_expression DIV unary_expression {
					$$ = hbb_create_binary_expression(DIV_EXPRESSION, $1, $3);
			}
			| multiplicative_expression MOD unary_expression {
					$$ = hbb_create_binary_expression(MOD_EXPRESSION, $1, $3);
			}
			;

unary_expression:
			primary_expression
			| SUB unary_expression {
					$$ = hbb_create_minus_expression($2);
			}
			;

primary_expression:
			IDENTIFIER LP argument_list RP {
					$$ = hbb_create_function_call_expresssion($1, $3);
			}
			| IDENTIFIER LP RP {
					$$ = hbb_create_function_call_expresssion($1, NULL);
			}
			| LP expression RP {
					$$ = $2;
			}
			| IDENTIFIER {
				 	$$ = hbb_create_identifier_expression($1);
			}
			| INT_LITERAL
			| DOUBLE_LITERAL
			| STRING_LITERAL
			| TRUE_T {
					$$ = hbb_create_boolean_expression(CRB_TRUE);
			}
			| FALSE_T {
					$$ = hbb_create_boolean_expression(CRB_FALSE);
			}
			| NULL_T {
					$$ = hbb_create_null_expression();
			}
			;

statement:
			expression SEMICOLON {
					$$ = hbb_create_expression_statement($1);
			}
			| global_statement
			| if_statement
			| while_statement
			| for_statement
			| return_statement
			| break_statement
			| continue_statement
			;

global_statement:
			GLOBAL_T identifier_list SEMICOLON {
					$$ = hbb_create_global_statement($2);
			}
			;

			/* id  */
identifier_list:
			IDENTIFIER {
					$$ = hbb_create_global_identifier($1);
			}
			| identifier_list COMMA IDENTIFIER {
					$$ = hbb_chain_identifier($1, $3);
			}
			;

			/* if(ex){} / if(ex){}else{} / if(ex){} elseif->... */
if_statement:
			IF LP expression RP block {
					$$ = hbb_create_if_statement($3, $5, NULL, NULL);
			}
			| IF LP expression RP block ELSE block {
					$$ = hbb_create_if_statement($3, $5, NULL, $7);
			}
			| IF LP expression RP block elseif_list {
					$$ = hbb_create_if_statement($3, $5, $6, $8);
			}
			;

			/* elseif / ex->elseif */
elseif_list:
			elseif
			| elseif_list elseif {
					$$ = hbb_chain_elseif_list($1, $2);
			}
			;

			/* elseif (ex) {} */
elseif:
			ELSEIF LP expression RP block {
					$$ = hbb_create_elseif($3, $5);
			}
			;

			/* while(ex){} */
while_statement:
			WHILE LP expression RP block {
					$$ = hbb_create_while_statement($3, $5);
			}
			;

			/* for(ex;ex;ex){} */
for_statement:
			FOR LP expression_opt SEMICOLON expression_opt SEMICOLON
			 expression_opt RP block {
				 	$$ = hbb_create_for_statement($3, $5, $7, $9);
			}
			;

			/* null / expression */
expression_opt: {
					/* empty */
					$$ = NULL;
			}
			| expression
			;

			/* return expression_opt; */
return_statement:
			RETURN_T expression_opt SEMICOLON {
					$$ = hbb_create_return_statement($2);
			}
			;

			/* break; */
break_statement:
			BREAK SEMICOLON {
					$$ = hbb_create_continue_statement();
			}
			;

			/* continue; */
continue_statement:
			CONTINUE SEMICOLON {
					$$ = hbb_create_continue_statement();
			}
			;

			/* create block 1.{block} 2.{null block}*/
block:
			LC statement_list RC {
					$$ = hbb_create_block($2);
			}
			| LC RC {
				  $$ = hbb_create_block(NULL);
			}
			;

%%
