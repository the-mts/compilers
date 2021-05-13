# CS335 Compiler: FAAST

This is a compiler made as our CS335A (Semester II 2020-21) course project. The source language for the input files is ANSI C. The target language for the compiler is x86-64. The compiler is implemented in C/C++ language.

## Description

This is a basic compiler for compiling C files that adhere to the ANSI standard (C89). It supports almost all the standard ANSI C features, along with some code optimization, library functions (math and string), and basic file input/output features.

The entire source code is present in the `src` directory. It contains files to generate lexer, parser, and the final binary for the compiler. On building, the independent object files and the final binary (`faast`) are created in the `bin` directory. On compiling a program, the compiler will generate the binary, along with generating multiple files useful for debugging in the `bin` directory, such as `AST.dot` (dump of the AST), `symtab.csv` (dump of the symbol table), optional `typtab.csv` (dump of the user-defined types table), `3AC.txt` (dump of the intermediate 3 Address Code) and `assembly.s` (the compiled assembly code).

## Steps to Build and Run

Go to the main directory. To build the binary, simply run the __make__ command.

```console
$ make
```

To run a given test program, run the binary file, with the test file path specified as a command line parameter, followed by any valid options if needed. Note that the name for the output can be given using the `-o` option (explained in the following section).

```
$ ./bin/faast <file_name> [-options...]
```
  This will generate the binary output file with the given name (if no name given, it defaults to `./a.out`). Also, this will dump the supporting debug files in the `bin` directory.

## Options

The compiler supports the following options for purposes of debugging (and to disable the beta features if required).

* `-o <output_name>`: This option allows you to give a custom name to the generated binary for the input program. If no name is supplied, the binary is created as `./a.dot`.

* `-g`: This will dump the entire parse log in `STDERR`, to help visualize the parsing process (the rules used in reduction of the input file).

* `-t`: This will dump the types table for the program in `./bin/typtab.csv`, which will contain the information for the user-defined types in the program (`struct` and `union` supported).

* `-W`: This option will ***disable*** warnings about implicit type-casting required in the program.

* `-f`: This will ***disable*** all features related to file I/O handling in the compiler, along with disabling the data type for file pointers FILEP. This may be required since the feature has not been rigorously tested and may create problems for programs that should have compiled correctly otherwise.

* `-O`: This option will ***disable*** all code otimizations in the intermediate code. Again, this may be needed as optimization has not been exhaustively tested.

* `-T`: A basic form of tail call optimization has also been implemented by us, but this too, has not been tested thorougly. This option may thus be used to ***disable*** said feature.

## Makefile Clean Commands

We have provided 3 clean options in the makefile.
* `make clean`: This command will remove all the files created for building the final compiler executable (including all the `.o` files).

* `make fileclean`: This command will clean all the output files created during the compilation of a program (including the supporting debug files in `bin`).

* `make realclean`: This command will perform the above two tasks along with deleting the created compiler binary file `faast`.
