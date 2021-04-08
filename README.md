# CS335 Milestone 3

This is a standalone parser and semantic analyser for our CS335 Compilers project. It parses a given program (with the source language C) and generates a DOT file of the Abstract Syntax Tree. It also dumps the symbol table and can also dump the types table if the option is enabled.

## Description

The repository develops over the previously built parser. Additional files required for semantic analysis have been created. The file `parser.y` in `src` directory is a yacc file that specifies the grammar and actions that construct the AST. The directory also contains `symtab_utils.c` and `symtab.h` that provide basic utility functions to create and modify the symbol table and types table. The file `main.c` contains the driver code that takes the input file, calls the parser over it and stores the generated AST in a `DOT` file. It also dumps the symbol table in the file `symtab.csv`. The output consists of three columns - the name of the variable, its size and its type. Each scope/function is demarcated into separate sections, named by the function name, with the scopes inside the function numbered from 1 in the depth-first order as they appear in the input program. If types table dump is enabled, the output is dumped in the file `typtab.out` in the `bin` directory.

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
This will generate the `DOT` file with the given name. Also, this will dump the symbol table for the program in the file `./bin/symtab.csv`.

To visualize the AST graph, use the `dot` package and give the generated `DOT` file as input. This will generate a PostScript file of the graph.

```console
$ dot -Tps dotname.dot -o graph.ps
```

To view the generated PostScript file of the graph, use any PS viewer such as Evince.

```console
$ evince a.ps 
```

## Options

We have currently provided two options for the purpose of debugging.

* To view the entire parsing process (the rules used in reduction of the input), add the `-g` option as follows. This will dump the parse log in `STDERR`.
  ```console
  $ ./bin/parser ./tests/testfile.c -o dotname.dot -g
  ```

* To dump the table containing all the type definitions within each scope, add the `-t` option. This will generate the type table in the file `./bin/typtab.out` similar to the symbol table.
  ```console
  $ ./bin/parser ./tests/testfile.c -o dotname.dot -t
  ```
