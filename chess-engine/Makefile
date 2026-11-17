CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic

TARGET = ppchess
OBJECTS = src/main.o src/board.o src/movegen.o src/search.o
HEADERS = $(wildcard include/*.h)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/perft.o: tests/perft.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: tests/perft.o
	@echo "Tests are not implemented; perft is a placeholder. No tests passed."
	@exit 1

clean:
	$(RM) $(TARGET) $(OBJECTS) tests/perft.o
