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
