CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic

TARGET = ppchess
OBJECTS = src/main.o src/board.o src/movegen.o src/search.o
HEADERS = $(wildcard include/*.h)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/perft: tests/perft.c src/board.o src/movegen.o $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ tests/perft.c src/board.o src/movegen.o

test: tests/perft
	./tests/perft

clean:
	$(RM) $(TARGET) $(OBJECTS) tests/perft
