CC=gcc
CFLAGS+=-fno-omit-frame-pointer -g -Wall -std=c11
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=build/g0/%.o)
G1_ASMS=$(SRCS:src/%.c=build/g1/%.s)
# G1_OBJS=$(SRCS:src/%.c=build/g1/%.o)
G2_ASMS=$(SRCS:src/%.c=build/g2/%.s)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(CFLAGS) $(LDFLAGS)

build/g0/%.o : src/%.c src/hoc.h
	$(CC) $(CFLAGS) -c $< -o $@

build/g1/%.s: src/%.c hoc
	./hoc $< > $@

# build/g1/%.o: $(G1_ASMS)
# 	nasm -felf64 $<

# build_g1: $(G1_OBJS)
# 	$(CC) -g -static -o build/g1/hoc $(G1_OBJS) $(CFLAGS) $(LDFLAGS)

build_g1: $(G1_ASMS)
	$(CC) -g -static -o build/g1/hoc $(G1_ASMS) $(CFLAGS) $(LDFLAGS)

build/g2/%.s: src/%.c hoc
	./build/g1/hoc $< > $@

build_g2: $(G2_ASMS)
	$(CC) -g -static -o build/g2/hoc $(G2_ASMS) $(CFLAGS) $(LDFLAGS)

test: hoc build_g1 build_g2 FORCE
	./test.sh
	./hoc test/pp_test.c > test/pp_test.s
	$(CC) -g -static -o test/pp_test.out test/pp_test.s
	./test/pp_test.out

ir_test: hoc FORCE
	gcc -I./include -D__hoc__ -E -P test/test.c > test/tmp.c
	./hoc -i test/tmp.c > test/tmp_hoc_i.s
	nasm -felf64 test/tmp_hoc_i.s
	gcc -static -o test/tmp_hoc_i.out test/tmp_hoc_i.o
	./test/tmp_hoc_i.out

clean:
	$(RM) hoc $(OBJS) $(G1_ASMS) $(G2_ASMS) build/g1/hoc build/g2/hoc
	$(RM) test/pp_test.out

FORCE:
.PHONY: clean FORCE
