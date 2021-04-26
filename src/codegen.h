#ifndef CODEGEN_H
#define CODEGEN_H

#include<iostream>
#include<vector>
#include<string>
#include<map>
#include "symtab.h"
#include "3AC.h"

extern vector<quad> code_array;
extern int nextquad;

extern vector<string> code;
extern unordered_map<int, string> gotoLabels;
extern void codegen();

#endif