CC = gcc
CFLAGS = -std=c2x -O2 -Wall -Wextra -pedantic -I./ -lcmocka

all: clean rradix-test

test: clean rradix-test
	@echo "----- Running standard tests... -----"
	@./rradix-test

rradix-test: rradix.c rradix.h tests.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f rradix-test

.PHONY: all test
