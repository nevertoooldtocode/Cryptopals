# Makefile

CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra

SRC     := src
INC     := include
TEST    := tests

PROD_SRCS := $(SRC)/main.c $(SRC)/board.c
TEST_SRCS := $(TEST)/test_board.c

PROD_BIN  := Chess
TEST_BIN  := Test_Chess

.PHONY: all production test run-tests clean

all: production test run-tests

# Build production executable

production: $(PROD_SRCS)
	$(CC) $(CFLAGS) $^ -o $(PROD_BIN)

# Note: $^ stands for the names of the prerequisites separated by spaces. In this case $(PROD_SRCS)

# Build test runner (includes math_utils.c + test file)

test: $(SRC)/board.c $(TEST_SRCS)
	$(CC) $(CFLAGS) $^ -o $(TEST_BIN)

# Note: $^ stands for the names of the prerequisites separated by spaces. In this case $(SRC)/math_utils.c $(TEST_SRCS)

# Alias to compile & immediately run tests

run-tests: test
	./$(TEST_BIN)

run: production
	./$(PROD_BIN)

clean:
	rm -f $(PROD_BIN) $(TEST_BIN)

# Usage:
# 
# $ make               # builds both math_app and test_runner
# $ ./math_app         # runs production app (if main.c exists)
# $ make run-tests     # builds & runs all unit tests
# $ make clean         # remove binaries
