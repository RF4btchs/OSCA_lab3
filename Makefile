CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -O2
BIN     = bin
SRC     = src/main.c src/search.c src/util.c

all: $(BIN)/lab3.exe

$(BIN)/lab3.exe: $(SRC) src/search.h src/util.h | $(BIN)
	$(CC) $(CFLAGS) -o $@ $(SRC)

# папка создаётся, только если её ещё нет (иначе make -B падает)
$(BIN):
	$(if $(wildcard $(BIN)),,mkdir $(BIN))

clean:
	-del /q $(BIN)\*.exe

.PHONY: all clean
