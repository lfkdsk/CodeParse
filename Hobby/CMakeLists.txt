cmake_minimum_required(VERSION 3.5)
project(Hobby)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp lex/Lexer.cpp lex/Lexer.h word/Token.cpp word/Token.h word/Word.cpp word/Word.h word/Tag.h word/BasicType.cpp word/BasicType.h Id/Integer.cpp Id/Integer.h Id/Real.cpp Id/Real.h parser/Parser.cpp parser/Parser.h utils/Printer.cpp utils/Printer.h parser/Node.cpp parser/Node.h Id/Character.cpp Id/Character.h Id/Double.cpp Id/Double.h)
add_executable(Hobby ${SOURCE_FILES})