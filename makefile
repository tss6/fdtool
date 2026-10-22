CC = cc
CFLAGS = -Iinclude -std=c11 -Wall -Wextra -g

TARGET = fdtool
OBJS = build/main.o build/diag.o build/lexer.o build/source.o

$(TARGET): $(OBJS)
	$(CC) -o $@ $^

build:
	mkdir -p build

build/main.o: main.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@
