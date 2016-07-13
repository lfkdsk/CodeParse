//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_PARSER_H
#define HOBBY_PARSER_H


#include "../lex/Lexer.h"

class Parser {
private:
    Lexer *lexer = nullptr;
    Token *lookahead;
    Printer printer;
public:

    Parser(Lexer *lexer);

    ~Parser();

    void move();

    void match(const int ch);

    void program();


};


#endif //HOBBY_PARSER_H
