#ifndef CODEGEN_H
#define CODEGEN_H

#include "3AC.h"

extern vector<quad> code_array;
extern int nextquad = 0;

extern vector<string> code;
extern unordered_map<int, string> gotoLabels;

#endif