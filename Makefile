CC=gcc
#CFLAGS=-Wall -Wpedantic -O2
CFLAGS=-Wall -Wpedantic -g -O0
IFLAGS=-Isrc

SOURCES:=$(shell find src -path "src/heads" -prune -o -name "*.[ch]" -print)
FRAGMENTS:=$(shell find src -path "src/heads" -prune -o -name "*.[ch]f" -print)
ALL_SOURCES:=$(SOURCES) $(FRAGMENTS)
HEADS:=$(shell find src/heads -name "*.c" -print)

.PHONY: list
list:
	@echo $(ALL_SOURCES)
	@echo $(HEADS)

bin/test: $(ALL_SOURCES) src/heads/test.c
	$(CC) $(CFLAGS) $(IFLAGS) $(SOURCES) src/heads/test.c -o $@

bin/rng: $(ALL_SOURCES) src/heads/rng.c
	$(CC) $(CFLAGS) $(IFLAGS) $(SOURCES) src/heads/rng.c -o $@

.PHONY: test
test: bin/test
	./bin/test

.PHONY: rng
rng: bin/rng
	./bin/rng 1000

.PHONY: clean
clean:
	rm bin/*
