//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_CHAR_H
#define HOBBY_CHAR_H


#include "../word/Token.h"

class Character : public Token {

public:
    Character(const char* val) : Token(CHAR) {
        this->value = *val;
    }


    char getValue() const {
        return value;
    }

    void setValue(char value) {
        Character::value = value;
    }

private:
    char value;
};


#endif //HOBBY_CHAR_H
