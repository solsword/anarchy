CC=gcc
#CFLAGS=-Wall -Wpedantic -O2
CFLAGS=-Wall -Wpedantic -g -O0
IFLAGS=-Isrc

bin/test: src/test.c
	$(CC) $(CFLAGS) $(IFLAGS) $^ -o $@

.PHONY: run
run: bin/test
	./bin/test

.PHONY: clean
clean:
	rm bin/*
