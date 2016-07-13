//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_FLOAT_H
#define HOBBY_FLOAT_H

#include "../word/Token.h"

class Real : public Token {

public:
    Real(float val) : Token(REAL) {
        this->value = val;
    }

    float getValue() const {
        return value;
    }


    void setValue(float value) {
        this->value = value;
    }

private:
    float value;
};


#endif //HOBBY_FLOAT_H
