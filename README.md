# CS335 Lexer

This is a standalone lexical analyser for our CS335 Compilers project. It scans a given program (with the source language C) and prints all the individual tokens along with their matched lexemes and the line and column numbers.

## Description

The repository consists basically of the lex file `scanner.l` in the `src` directory. The makefile will first generate the `lex.yy.c` file and then compile it to the binary `scanner` in the `bin` directory. Our sample testcases are given in the directory `tests` with the names as `test<num>.c` (where `num` is from 1 to 5).

## Steps to build and run

Go to the main directory. To build the binary, simply run the __make__ command.

```console
$ make
```

To run a given test program, run the binary file, with the test file path specified as a command line parameter.

```console
$ ./bin/scanner ./tests/testfile.c
```
