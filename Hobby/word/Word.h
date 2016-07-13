//
// Created by 刘丰恺 on 16/6/4.
//

#ifndef HOBBY_WORD_H
#define HOBBY_WORD_H


#include "Token.h"
#include <string>

class Word : public Token {
private:
    std::string lexeme = "";
public:
    Word(std::string lexeme, int tag) : Token(tag) {
        this->lexeme = lexeme;
    }


    const std::string &getLexeme() const {
        return lexeme;
    }

    void setLexeme(const std::string &lexeme) {
        Word::lexeme = lexeme;
    }

};


#endif //HOBBY_WORD_H
