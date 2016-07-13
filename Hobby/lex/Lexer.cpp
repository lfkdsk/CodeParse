//
// Created by 刘丰恺 on 16/6/4.
//

#include "Lexer.h"

static Word
        *ifWord = new Word("if", IF),
        *trueWord = new Word("true", TRUE),
        *falseWord = new Word("false", FALSE),
        *elseWord = new Word("else", ELSE),
        *whileWord = new Word("while", WHILE),
        *doWord = new Word("do", DO),
        *breakWord = new Word("break", BREAK);

static Word
        *andWord = new Word("&&", AND),
        *orWord = new Word("||", OR),
        *eqWord = new Word("==", EQ),
        *neWord = new Word("!=", NE),
        *leWord = new Word("<=", LE),
        *geWord = new Word(">=", GE),
        *minWord = new Word("minus", MINUS),
        *temp = new Word("temp", TEMP);

static BasicType
        *Int = new BasicType("int", BASIC, 4),
        *Float = new BasicType("float", BASIC, 8),
        *Char = new BasicType("char", BASIC, 1),
        *Bool = new BasicType("bool", BASIC, 1),
        *Dou = new BasicType("double", BASIC, 16);

Lexer::Lexer() {
    // 装载词素
    addWordToTable(ifWord);
    addWordToTable(elseWord);
    addWordToTable(whileWord);
    addWordToTable(doWord);
    addWordToTable(breakWord);
    addWordToTable(trueWord);
    addWordToTable(falseWord);
    addWordToTable(Int);
    addWordToTable(Float);
    addWordToTable(Char);
    addWordToTable(Dou);
    addWordToTable(Bool);

}

Lexer::~Lexer() {
    word_table.clear();
    filein.close();
    delete andWord, orWord, eqWord, neWord, leWord, geWord, minWord, temp;
    delete Int, Float, Char, Bool;
    delete ifWord, trueWord, falseWord, elseWord, whileWord, doWord, breakWord;
}

void Lexer::addWordToTable(Word *word) {
    word_table[word->getLexeme()] = word;
}

void Lexer::printWordTable() {
    for (auto i = word_table.begin(); i != word_table.end(); ++i) {
        std::cout << "key::" << i->first << "  value ::"
        << i->second->getLexeme() << std::endl;
    }
}

void Lexer::nextChar() {
    int charInt = filein.get();
    if (charInt != EOF) {
        peekChar = (char) charInt;
        printer.println(peekChar);
    } else {
        printer.println("the file is over");
        peekChar = EOF;
        return;
    }
}

void Lexer::openFile(const std::string filename) {
    assert(isSupportFormat(filename));
    filein.open(filename);
    if (filein.fail()) {
        printer.println("open file " + filename + " fail");
        exit(EXIT_FAILURE);
    }
}

void Lexer::closeFile() {
    filein.close();
}

bool Lexer::isSupportFormat(string filename) {
    string suffix = filename.substr(filename.size() - 3, 3);
    return !(suffix != ".hy");
}

Token *Lexer::nextToken() {
    // 跳过空白行/字符/制表符
    for (; ; nextChar()) {
        if (peekChar == ' ' || peekChar == '\t')
            continue;
        else if (peekChar == '\n')
            lineNumber = lineNumber + 1;
        else break;
    }

    // 跳过注释
    handleInComment();

    Token *token;
    // 处理bool符号
    token = handleBool();
    if (token != nullptr) {
        return token;
    }

    // 处理数字
    if (isdigit(peekChar)) {
        return handleDigit();
    }

    // 处理ID的合成
    if (isalpha(peekChar)) {
        return handleID();
    }

    token = handleChar();
    if (token != nullptr) {
        return token;
    }

    token = new Token((int) peekChar);
    peekChar = ' ';
    return token;
}

/**
 * 检索双目运算符
 */
bool Lexer::isEqual(const char ch) {
    nextChar();
    if (peekChar != ch)
        return false;

    peekChar = ' ';
    return true;
}

/**
 * 判断一个单词的结束
 * 单词接受数字\字母\_
 * 开头一定是字母
 */
bool Lexer::isOneWord(const char ch) {
    return (peekChar != EOF && !isblank(ch)) &&
           (isdigit(peekChar) || isalpha(peekChar)
            || peekChar == '_');
}

Token *Lexer::handleDigit() {
    int v = 0;
    do {
        v = 10 * v + digittoint(peekChar);
        nextChar();
    } while (isdigit(peekChar));


    if (peekChar == 'e' | peekChar == 'E') {
        // 科学计数法
        nextChar();
        int x = 0;
        bool flag = false;

        if (peekChar == '-') {
            flag = true;
            nextChar();
        } else if (peekChar == '+') {
            flag = false;
            nextChar();
        }

        do {
            x = 10 * x + digittoint(peekChar);
            nextChar();
        } while (isdigit(peekChar));

        double val = v * exp(flag ? -x : x);
        Double *dou = new Double(val);
        return dou;
    } else if (peekChar != '.') {
        Integer *integer = new Integer(v);
        return integer;
    }

    float x = v;
    for (float d = 10; ;) {
        nextChar();
        if (!isdigit(peekChar))
            break;
        x = x + digittoint(peekChar) / d;
        d = d * 10;
    }
    Real *realNum = new Real(x);
    return realNum;
}

Token *Lexer::handleID() {
    string buffer = "";
    string *point = &buffer;
    do {
        point->append(&peekChar);
        nextChar();
    } while (isOneWord(peekChar));

    auto word_list = word_table.find(buffer);

    if (word_list != word_table.end()) {
        return word_list->second;
    }

    Word *w = new Word(buffer, ID);
    word_table[w->getLexeme()] = w;
    return w;
}

Token *Lexer::handleBool() {
    switch (peekChar) {
        case '&':
            if (isEqual('&')) return andWord;
            else return new Token('&');
        case '|':
            if (isEqual('|')) return orWord;
            else return new Token('|');
        case '=':
            if (isEqual('=')) return eqWord;
            else return new Token('=');
        case '!':
            if (isEqual('=')) return neWord;
            else return new Token('!');
        case '<':
            if (isEqual('=')) return leWord;
            else return new Token('<');
        case '>':
            if (isEqual('=')) return geWord;
            else return new Token('>');
        default:
            break;
    }
    return nullptr;
}

void Lexer::handleInComment() {
    if (peekChar == '/') {
        nextChar();
        if (peekChar == '/') {
            for (; ; nextChar()) {
                if (peekChar == '\n' || peekChar == EOF) {
                    lineNumber += 1;
                    return;
                }
            }
        }
        char lastChar = ' ';
        if (peekChar == '*') {
            for (; ; nextChar()) {
                if (peekChar == '\n') {
                    lineNumber += 1;
                } else if (peekChar == '/' && lastChar == '*') {
                    return;
                }
                lastChar = peekChar;
            }
        }
    }
}

Token *Lexer::handleChar() {
    if (peekChar == '\'') {
        bool flag = false;
        string *buffer = new string;
        char lastChar = ' ';
        for (; ; nextChar()) {
            if (peekChar == EOF) {
                printer.error(lineNumber, "cannot find the end of char");
            }

            if (peekChar == '\'' && flag
                && lastChar != '\\') {
                break;
            } else if (peekChar == '\n') {
                lineNumber += 1;
                continue;
            } else if (peekChar != '\'') {
                buffer->append(&peekChar);
            }
            lastChar = peekChar;
            flag = true;
        }
        Character *character = new Character(buffer->c_str());
        return character;
    }
    return nullptr;
}













































