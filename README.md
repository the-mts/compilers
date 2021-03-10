# CS335 Parser

This is a standalone parser for our CS335 Compilers project. It parses a given program (with the source language C) and generates a DOT file of the Abstract Syntax Tree, which can be viewed using a graph visualizer like Graphviz.

## Description

The repository develops over the previously built lexer. Additional files for parser have been created. The file `parser.y` in `src` directory is a yacc file that specifies the grammar and actions that construct the AST. The directory also contains `parse_utils.c` and `parse_utils.h` that provide basic utility functions to create and modify the tree nodes. The file `main.c` contains the driver code that takes the input file, calls the parser over it, prints the tree in the DOT specification format, and finally stores it in a DOT file.

The makefile will generate `y.tab.c` parser file, the corresponding header file, and the `lex.yy.c` scanner file. It will then compile all the source files to generate the `parser` binary in the `bin` directory. Our sample testcases are given in the directory `tests` with the names as `test<num>.c` (where `num` is from 1 to 5).

## Steps to build and run

Go to the main directory. To build the binary, simply run the __make__ command.

```console
$ make
```

To run a given test program, run the binary file, with the test file path specified as a command line parameter and specify the name of the `DOT` file using the `-o` option.

```console
$ ./bin/parser ./tests/testfile.c -o dotname.dot
```

To view the entire parsing process, add the `-g` option as follows. This will dump the parse log in `STDERR`.

```console
$ ./bin/parser ./tests/testfile.c -o dotname.dot -g
```

To visualize the AST graph, use the `dot` package and give the generated `DOT` file as input. This will generate a PostScript file of the graph.

```console
$ dot -Tps graph1.dot -o graph1.ps
```
