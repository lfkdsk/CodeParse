%{

double vbltable[26];

%}

%union{
    double dval;
    int vblno;
}

%token <vblno> NAME
%token <vblno> NUMBER
%left '-' '+'
%left '*' '/'
%nonassoc UMINUS

%type <dval> expression

%%

stament_list: stament '\n'
            | stament_list stament '\n'
            ;

stament: NAME '=' expression { vbltable[$1] = $3; }
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
          | NAME   { $$ = vbltable[$1]; }
          ;

%%
