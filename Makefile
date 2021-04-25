CC=g++
LEX=flex
YACC=bison -y
BIN=bin
SRC=src
.PHONY: all clean realclean fileclean
all: $(BIN)/parser

$(BIN)/parser: $(SRC)/lex.yy.c $(SRC)/y.tab.c $(BIN)/main.o $(BIN)/parse_utils.o $(BIN)/symtab_utils.o $(BIN)/3AC.o $(BIN)/codegen.o
	$(CC) -Wno-write-strings -Wno-abi $^ -o $@

$(SRC)/lex.yy.c: $(SRC)/scanner.l $(SRC)/y.tab.h $(SRC)/y.tab.c $(SRC)/symtab.h $(SRC)/parse_utils.h
	$(LEX) -o $@ $<

$(SRC)/y.tab.c $(SRC)/y.tab.h: $(SRC)/parser.y $(SRC)/parse_utils.h $(SRC)/symtab.h
	$(YACC) -dvt -Wno-yacc -Wno-other -o $(SRC)/y.tab.c $<

$(BIN)/main.o: $(SRC)/main.c $(SRC)/parse_utils.h $(SRC)/symtab.h $(SRC)/3AC.h $(SRC)/codegen.h
	$(CC) -c $< -o $@

$(BIN)/parse_utils.o: $(SRC)/parse_utils.c $(SRC)/parse_utils.h $(SRC)/3AC.h
	$(CC) -c $< -o $@

$(BIN)/symtab_utils.o: $(SRC)/symtab_utils.c $(SRC)/symtab.h
	$(CC) -c -Wno-abi $< -o $@

$(BIN)/3AC.o: $(SRC)/3AC.c $(SRC)/3AC.h
	$(CC) -c $< -o $@

$(BIN)/codegen.o: $(SRC)/codegen.c $(SRC)/codegen.h $(SRC)/3AC.h
	$(CC) -c $< -o $@

clean:
	$(RM) $(SRC)/lex.yy.c $(SRC)/y.tab.c $(SRC)/y.tab.h $(SRC)/y.output $(BIN)/*.o

fileclean:
	$(RM) $(BIN)/*.csv $(BIN)/*.out $(BIN)/*.dot $(BIN)/*.ps

realclean: clean fileclean
	$(RM) $(BIN)/parser
