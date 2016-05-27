%{

unsigned long charCount = 0, wordCount =0, lineCount = 0;

#undef yywrap
%}

word [^\t\n]+
eol  \n

%%

{word} {
          wordCount++;
          charCount += yyleng;
       }

{eol}  {
          charCount++;
          lineCount++;
       }

.      {
          charCount++;
       }

%%

char **fileList;
unsigned currentFile = 0;
unsigned nFiles;
unsigned long totalCC = 0,totalWC = 0,totalLC = 0;

int main(int argc,char **argv){
    FILE *file;
    // 文件名
    fileList = argv + 1;
    nFiles = argc - 1;

    if(argc == 2){
        /* 因为单文件不需要打印文件名和统计所以处理不同 */
        currentFile = 1;
        file = fopen(argv[1], "r");
        if(!file){
            fprintf(stderr, "could not open %s \n", argv[1]);
            exit(1);
        }
        yyin = file;
    }

    if(argc > 2){
        yywrap();  
    }

    yylex();

    if(argc > 2){
        printf("%8lu %8lu %8lu %s \n", lineCount, wordCount, charCount
            , fileList[currentFile -1]);
        totalCC += charCount;
        totalWC += wordCount;
        totalLC += lineCount;
        printf("%8lu %8lu %8lu \n", totalLC, totalWC, totalCC);
    }else{
        printf("%8lu %8lu %8lu \n", lineCount, wordCount, charCount);
    }

    return 0;
}

/* 词法分析程序 调用yywrap() 处理EOF 复写可解决连接问题 */
/* yywrap() 这个函数是在yylex()运行结尾进行调用
    复写这个函数就会重新提供1个文件给lex分析，就会继续进行
    通过返回值0/1判断有没有继续的内容
 */
int yywrap(){
    FILE *file = NULL;

    if((currentFile != 0) && (nFiles > 1) && (currentFile < nFiles)){
        /* 打印出前一个文件的统计信息 */
        printf("%8lu %8lu %8lu %s \n", lineCount, wordCount, charCount
            , fileList[currentFile -1]);
        totalCC += charCount;
        totalWC += wordCount;
        totalLC += lineCount;
        charCount = wordCount = lineCount = 0;
        fclose(yyin);
    }

    while(fileList[currentFile] != (char*)0){
        file = fopen(fileList[currentFile++], "r");
        if(file != NULL){
            yyin = file;
            break;
        }
        fprintf(stderr,"could not open %s \n", fileList[currentFile - 1]);
    }

    return (file ? 0 : 1); /* 0表示更多输入 */
}
