//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_INTEGER_H
#define HOBBY_INTEGER_H

#include "../word/Token.h"

class Integer : public Token {

public:

    Integer(int val) : Token(NUM) {
        this->value = val;
    }


    int getValue() const {
        return value;
    }


    void setValue(int value) {
        Integer::value = value;
    }

private:
    int value = 0;
};


#endif //HOBBY_INTEGER_H
