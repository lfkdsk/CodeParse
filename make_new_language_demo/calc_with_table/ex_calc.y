%{

#include "link_list.h"

%}

%union{
    double dval;
    struct symtable *symp;
}

%token <symp> NAME
%token <dval> NUMBER
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <dval> expression

%%

stament_list: stament '\n'
            | stament_list stament '\n'
            ;

stament: NAME '=' expression {
          $1->value = $3;
          printf("%s fuck",$1->name);
        }
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
          | NUMBER { $$ = $1;
                      printf("%f fuck \n",$1);
                   }
          | NAME   { $$ = $1->value; }
          ;

%%
