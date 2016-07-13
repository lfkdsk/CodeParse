//
// Created by 刘丰恺 on 16/6/5.
//

#include "Printer.h"

void Printer::println(const string line) {
    std::cout << line << std::endl;
}

void Printer::println(const char ch) {
    std::cout << ch << std::endl;
}

void Printer::error(int lineNum, const string line) {
    throw ("near line " + line + ": " + line);
}



