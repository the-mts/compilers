CC=gcc
LEX=flex
YACC=bison -y
BIN=bin
SRC=src
.PHONY: all clean realclean
all: $(BIN)/parser

$(BIN)/parser: $(SRC)/lex.yy.c $(SRC)/y.tab.c $(BIN)/main.o $(BIN)/parse_utils.o
	$(CC) -lfl $^ -o $@

$(SRC)/lex.yy.c: $(SRC)/scanner.l $(SRC)/y.tab.h $(SRC)/y.tab.c
	$(LEX) -o $@ $<

$(SRC)/y.tab.c $(SRC)/y.tab.h: $(SRC)/parser.y
	$(YACC) -dvt -Wno-conflicts-sr -o $(SRC)/y.tab.c $<

$(BIN)/main.o: $(SRC)/main.c $(SRC)/parse_utils.h
	$(CC) -c $< -o $@

$(BIN)/parse_utils.o: $(SRC)/parse_utils.c $(SRC)/parse_utils.h
	$(CC) -c $< -o $@

clean:
	$(RM) $(SRC)/lex.yy.c $(SRC)/y.tab.c $(SRC)/y.tab.h $(SRC)/y.output $(BIN)/parse_utils.o $(BIN)/main.o

realclean: clean
	$(RM) $(BIN)/scanner
