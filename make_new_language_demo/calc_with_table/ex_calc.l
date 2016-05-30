%{

#include "y.tab.h"
#include <math.h>
#include "link_list.h"

%}

%%

([0-9]+|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?) {
          yylval.dval = atof(yytext);
          printf("number : %s \n", yytext);
          return NUMBER;
    }

[ \t] ;

[A-Za-z][A-Za-z0-9]* {
          yylval.name = addNode(yytext);
          printLink();
          return NAME;
      }

"$"   {
          return 0;
      }

\n    |

.     {
            return yytext[0];
      }

%%
