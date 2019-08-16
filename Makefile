CFLAGS=-g -Wall -std=c11 -I./include
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=build/g0/%.o)
G1_ASMS=$(SRCS:src/%.c=build/g1/%.s)
G2_ASMS=$(SRCS:src/%.c=build/g2/%.s)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

build/g0/%.o : src/%.c include/hoc.h
	$(CC) $(CFLAGS) -c $< -o $@

build/g1/%.s: src/%.c hoc
	./hoc $< > $@

build_g1: $(G1_ASMS)
	$(CC) -g -static -o build/g1/hoc $(G1_ASMS) $(LDFLAGS)

build/g2/%.s: src/%.c hoc
	./build/g1/hoc $< > $@

build_g2: $(G2_ASMS)
	$(CC) -g -static -o build/g2/hoc $(G2_ASMS) $(LDFLAGS)

test: hoc build_g1 build_g2 FORCE
	./test.sh
	./hoc test/pp_test.c > test/pp_test.s
	gcc -g -static -o test/pp_test.out test/pp_test.s
	./test/pp_test.out

clean:
	$(RM) hoc $(OBJS) $(G1_ASMS) $(G2_ASMS) build/g1/hoc build/g2/hoc
	$(RM) test/pp_test.out

FORCE:
.PHONY: clean FORCE
