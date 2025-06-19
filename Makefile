CC = gcc
CFLAGS = -std=c2x -O2 -Wall -Wextra -pedantic -I./ -lcmocka -fsanitize=address -fno-omit-frame-pointer

all: clean rradix-test

test: clean rradix-test
	@echo "----- Running standard tests... -----"
	@./rradix-test

test-debug: clean rradix-test-debug
	@echo "----- Running debug tests... -----"
	@./rradix-test-debug

rradix-test: rradix.c rradix.h tests.c
	$(CC) -o $@ $^ $(CFLAGS)

rradix-test-debug: rradix.c rradix.h tests.c
	$(CC) -o $@ $^ $(CFLAGS) -DDEBUG

clean:
	rm -f rradix-test rradix-test-debug

.PHONY: all test
