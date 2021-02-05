CC=gcc
LEX=flex
YACC=bison -y
BIN=bin
SRC=src
.PHONY: all clean realclean
all: $(BIN)/scanner

$(BIN)/scanner: $(SRC)/lex.yy.c
	$(CC) -lfl $(SRC)/lex.yy.c -o $@

$(SRC)/lex.yy.c: $(SRC)/scanner.l
	$(LEX) -o $@ $(SRC)/scanner.l

clean:
	$(RM) $(SRC)/lex.yy.c

realclean: clean
	$(RM) $(BIN)/scanner
