//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_DOUBLE_H
#define HOBBY_DOUBLE_H


#include "../word/Token.h"

class Double : public Token {

public:
    Double(double val) : Token(DOUBLE) {
        this->value = val;
    }

    double getValue() const {
        return value;
    }

    void setValue(double val) {
        Double::value = val;
    }

private:
    double value;
};


#endif //HOBBY_DOUBLE_H
