#ifndef SYMBOL
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
struct symtable {
  /* data */
    char *name;
    double value;
    struct symtable *next;
};

struct symtable* addNode(char *symbol);
void printLink();
#endif
