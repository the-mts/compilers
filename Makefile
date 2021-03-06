CC=gcc
LEX=flex
YACC=bison -y
BIN=bin
SRC=src
.PHONY: all clean realclean
all: $(BIN)/parser

$(BIN)/parser: $(SRC)/lex.yy.c $(SRC)/y.tab.c
	$(CC) -lfl $^ $(SRC)/main.c $(SRC)/parse_utils.c  -o $@

$(SRC)/lex.yy.c: $(SRC)/scanner.l $(SRC)/y.tab.h $(SRC)/y.tab.c
	$(LEX) -o $@ $<

$(SRC)/y.tab.c $(SRC)/y.tab.h: $(SRC)/parser.y
	$(YACC) -dvt -o $(SRC)/y.tab.c $<

clean:
	$(RM) $(SRC)/lex.yy.c $(SRC)/y.tab.c $(SRC)/y.tab.h

realclean: clean
	$(RM) $(BIN)/scanner
