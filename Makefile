# Makefile

CC      := gcc
CFLAGS  := -Iinclude -Wall -Wextra

SRC     := src
INC     := include
TEST    := tests
BIN	:= bin
ARGS	:= 6.txt

SUPPORT_SRCS := $(SRC)/bitarray.c $(SRC)/aes.c
PROD_SRCS := $(SRC)/Ch6.c $(SUPPORT_SRCS)
TEST_SRCS := $(TEST)/test_cryptopals.c

PROD_BIN  := $(BIN)/Ch6
TEST_BIN  := $(BIN)/test_cryptopals

.PHONY: all production test run-tests clean

all: production 

# Build production executable

production: $(PROD_SRCS)
	$(CC) $(CFLAGS) $^ -o $(PROD_BIN)

# Note: $^ stands for the names of the prerequisites separated by spaces. In this case $(PROD_SRCS)

test: $(SUPPORT_SRCS) $(TEST_SRCS)
	$(CC) $(CFLAGS) $^ -o $(TEST_BIN)

run-tests: test
	./$(TEST_BIN)

run: production test
	./$(TEST_BIN)
	./$(PROD_BIN) $(ARGS)

clean:
	rm -f $(PROD_BIN) $(TEST_BIN)

# Usage:
# 
# $ make               # builds both math_app and test_runner
# $ make run-tests     # builds & runs all unit tests
# $ make clean         # remove binaries
