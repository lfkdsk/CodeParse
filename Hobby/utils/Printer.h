//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_PRINTER_H
#define HOBBY_PRINTER_H

#include <iostream>

using namespace std;

class Printer {
public:
    Printer() { };

    ~Printer() { };

    void println(const string line);

    void println(const char ch);

    void error(const int lineNum, const string line);
};


#endif //HOBBY_PRINTER_H
