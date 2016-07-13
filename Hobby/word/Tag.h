//
// Created by 刘丰恺 on 16/6/5.
//

#ifndef HOBBY_TAG_H
#define HOBBY_TAG_H

/*
 * Tag标示符对应的数字编码
 * 255+的编码值防止出现奇怪的问题
 */

static enum Tag {
    AND = 256, BASIC = 257, BREAK = 258, DO = 259, ELSE = 260,
    EQ = 261, FALSE = 262, GE = 263, ID = 264, IF = 265,
    INDEX = 266, LE = 267, MINUS = 268, NE = 269, NUM = 270,
    OR = 271, REAL = 272, TEMP = 273, TRUE = 274, WHILE = 275,
    CHAR = 276, DOUBLE = 277
};

static enum NodeType {

};


#endif //HOBBY_TAG_H
