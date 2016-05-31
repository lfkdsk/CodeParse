%{

#include "link_list.h"
int yydebug=1;
#include <math.h>

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
          | NAME '(' expression ')' {
              if ($1->funcptr)
              {
                $$ = ($1->funcptr)($3);
              }else{
                printf("%s is not a function \n", $1->name);
                $$ = 0.0;
              }
          }
          ;

%%

int main(){

    extern double sqrt(), exp(), log();

    addFunction("sqrt", sqrt);
    addFunction("exp",exp);
    addFunction("log",log);

    yyparse();
}
