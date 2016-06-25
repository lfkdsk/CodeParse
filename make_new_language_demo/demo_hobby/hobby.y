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
	RQ NR GT GE LT LR ADD SUB MUL DIV MOD TURE_T FALSE_T GLOBAL_T

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

function_definition:
			FUNCTION IDENTIFIER LP parameter_list RP block {
					hbb_function_define($2 ,$4, $6);
			}
			| FUNCTION IDENTIFIER LP RP block {
					hbb_function_define($2 , NULL, $5);
			}
			;

parameter_list:
			IDENTIFIER {
					$$ = hbb_create_parameter($1);
			}
			| parameter_list COMMA IDENTIFIER {
					$$ = hbb_chain_parameter($1, $3);
			}
			;

argument_list:
			expression {
					$$ = hbb_create_argument_list($1);
			}
			| argument_list COMMA expression {
					$$ = hbb_chain_argument_list($1, $3);
			}
			;

statement_list:
			statement {
					$$ = hbb_create_statement_list($1);
			}
			| statement_list statement {
					$$ = hbb_chain_statement_list($1, $2);
			}
			;

expression:
			logical_or_expression
			| IDENTIFIER ASSIGN expression {
					$$ = hbb_create_assign_expression($1, $3);
			}
			;

logical_or_expression:
			logical_and_expression
			| logical_or_expression LOGICAL_OR logical_and_expression {
					$$ = hbb_create_binary_expression(LOGICAL_OR_EXPRESSION, $1, $3);
			}
			;

logical_and_expression:
			equiality_expression
			| logical_and_expression LOGICAL_AND equiality_expression {
					$$ = hbb_create_binary_expression(LOGICAL_AND_EXPRESSION, $1 , $3);
			}
			;

equiality_expression:
			relational_expression
			| equiality_expression GT relational_expression {
					$$ = hbb_create_binary_expression(equiality_expression, $1, $3);
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
			;
