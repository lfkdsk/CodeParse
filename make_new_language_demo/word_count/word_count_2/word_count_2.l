%{

#include <stdio.h>
unsigned charCount = 0, wordCount = 0, lineCount = 0;

%}

    /* 这里是一个替换的规则  */
    /* 不是 空格、制表符、换行符的非空内容 */
word [^ \t\n]+
eol  \n

%%

    /* 规则区 这里面的注释不能顶格写 */
    /* yyleng 是拿到的输入长度 */
{word} {
          wordCount++;
          charCount += yyleng;
       }

{eol}  {
         charCount++;
         lineCount++;
       }
    /* 匹配其他字符 */
.      {
         charCount++;
       }

%%

int main(int argc, char** argv){
    if(argc > 1){
        FILE *file;

        file = fopen(argv[1], "r");
        if(!file){
            fprintf(stderr, "could not open %s \n", argv[1]);
            exit(1);
        }
        yyin = file;
    }

    yylex();
    printf("%d %d %d \n", lineCount, wordCount, charCount);
    return 0;
}
