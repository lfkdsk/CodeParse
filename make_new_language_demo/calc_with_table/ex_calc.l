%{

#include "y.tab.h"
#include <math.h>
#include "link_list.h"

%}

%%

([0-9]+|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?) {
          yylval.dval = atof(yytext);
          return NUMBER;
    }

[ \t] ;

[A-Za-z][A-Za-z0-9]* {
          yylval.symp = addNode(yytext);
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
