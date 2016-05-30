%{

#include "link_list.h"
int yydebug=1;

%}

%union{
    double dval;
    struct symtable *name;
}

%token <name> NAME
%token <dval> NUMBER
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <dval> expression

%%

stament_list: stament '\n'
            | stament_list stament '\n'
            ;

stament: NAME '=' expression { $1->value = $3; }
       | expression { printf("= %g \n", $1); }
       ;

expression: expression '+' expression { $$ = $1 + $3; }
          | expression '-' expression { $$ = $1 - $3; }
          | expression '*' expression { $$ = $1 * $3; }
          | expression '/' expression {
              if($3 == 0.0){
                  yyerror("devide by zero");
              }else{
                  $$ = $1 / $3;
              }
          }
          | '-' expression %prec UMINUS { $$ = -$2; }
          | '(' expression ')' { $$ = $2; }
          | NUMBER
          | NAME   { $$ = $1->value; }
          ;

%%
