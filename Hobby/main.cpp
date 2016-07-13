#include "lex/Lexer.h"

int main(int argc, const char *argv[]) {
    Lexer lexer;
    lexer.openFile("/Users/liufengkai/Documents/JavaProject/Hobby/lfkdsk.hy");
    lexer.nextToken();
    lexer.nextToken();
    lexer.nextToken();
    lexer.nextToken();
    lexer.nextToken();
    lexer.nextToken();
    lexer.nextToken();

    lexer.printWordTable();
    return 0;
}