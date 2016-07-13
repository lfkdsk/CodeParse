//
// Created by 刘丰恺 on 16/6/4.
//

#ifndef HOBBY_LEXER_H
#define HOBBY_LEXER_H

#include <iostream>
#include <fstream>
#include "../word/Word.h"
#include "../word/BasicType.h"
#include "map"
#include <cassert>
#include "../Id/Integer.h"
#include "../Id/Real.h"
#include "../Id/Double.h"
#include "../utils/Printer.h"
#include <cctype>
#include "../Id/Character.h"
#include <cmath>


using namespace std;


class Lexer {
private:
    // 记录行号
    int lineNumber = 1;
    // 预读字符
    char peekChar = ' ';
    // 单词表
    map<const std::string, Word *> word_table;

    ifstream filein;

    Printer printer;

public:
    Lexer();

    ~Lexer();

    void openFile(const std::string filename);

    void closeFile();

    // 添加单词
    void addWordToTable(Word *word);

    void printWordTable();

    void nextChar();

    bool isEqual(const char ch);

    bool isOneWord(const char ch);

    Token *nextToken();

    Token *handleDigit();

    Token *handleID();

    Token *handleBool();

    Token *handleChar();

    void handleInComment();

    int getLineNumber() const {
        return lineNumber;
    }

    char getPeekChar() const {
        return peekChar;
    }

    bool isSupportFormat(string filename);
};


#endif //HOBBY_LEXER_H
