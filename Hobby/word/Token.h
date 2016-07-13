//
// Created by 刘丰恺 on 16/6/4.
//

#ifndef HOBBY_TOKEN_H
#define HOBBY_TOKEN_H

#include "Tag.h"
class Token {
private:
    int tagNum;

public:
    Token(int tag) {
        this->tagNum = tag;
    }

    ~Token(){

    }

    int getTagNum() const {
        return tagNum;
    }

    void setTagNum(int tagNum) {
        Token::tagNum = tagNum;
    }
};


#endif //HOBBY_TOKEN_H
