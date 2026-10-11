CC = clang
CPPFLAGS = -Iinclude
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
LDLIBS = -lm
PYTHON = python3

PROGRAM = build/electrocalc
UNIT_TEST = build/test_core
SOURCES = main.c src/cli.c src/input.c src/ohm.c
HEADERS = include/cli.h include/input.h include/ohm.h

.PHONY: all run test

all: $(PROGRAM)

build:
	mkdir -p build

$(PROGRAM): $(SOURCES) $(HEADERS) Makefile | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) $(LDFLAGS) $(LDLIBS) -o $@

$(UNIT_TEST): tests/test_core.c src/input.c src/ohm.c $(HEADERS) Makefile | build
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_core.c src/input.c src/ohm.c $(LDFLAGS) $(LDLIBS) -o $@

run: $(PROGRAM)
	./$(PROGRAM)

test: $(PROGRAM) $(UNIT_TEST)
	./$(UNIT_TEST)
	$(PYTHON) tests/test_cli.py ./$(PROGRAM)
