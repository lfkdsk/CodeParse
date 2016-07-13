//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_BASICTYPE_H
#define HOBBY_BASICTYPE_H


#include "Word.h"

class BasicType : public Word {
private:
    int wordWidth = 0;
public:
    BasicType(const std::string &lexeme, int tag, int width) : Word(lexeme, tag) {
        this->wordWidth = width;
    }


    int getWordWidth() const {
        return wordWidth;
    }

    void setWordWidth(int wordWidth) {
        BasicType::wordWidth = wordWidth;
    }
};


#endif //HOBBY_BASICTYPE_H
