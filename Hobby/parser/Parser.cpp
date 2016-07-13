//
// Created by 刘丰恺 on 16/6/5.
//

#include "Parser.h"

Parser::Parser(Lexer *lexer) {
    this->lexer = lexer;
    move();
}

Parser::~Parser() {
    delete lexer;
    delete lookahead;
}

void Parser::move() {
    lookahead = this->lexer->nextToken();
}

void Parser::match(const int ch) {
    if (lookahead->getTagNum() == ch) {
        move();
    } else {
        printer.error(lexer->getLineNumber(), "syntax error");
    }
}

void Parser::program() {

}






