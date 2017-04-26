CC=gcc
#CFLAGS=-Wall -Wpedantic -O2
CFLAGS=-Wall -Wpedantic -g -O0

bin/test: src/*
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: run
run: bin/test
	./bin/test
