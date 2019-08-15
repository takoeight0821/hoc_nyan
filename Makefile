CFLAGS=-g -Wall -std=c11 -I./include
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -static -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): include/hoc.h

selfhost:
	./hoc.sh src/containers.c
	$(CC) -g -c src/containers.s -o gen_first/containers.o
	./hoc.sh src/main.c
	$(CC) -g -c src/main.s -o gen_first/main.o
	./hoc.sh src/token.c
	$(CC) -c src/token.s -o gen_first/token.o
	./hoc.sh src/sema.c
	$(CC) -g -c src/sema.s -o gen_first/sema.o
	./hoc.sh src/parse.c
	$(CC) -c src/parse.s -o gen_first/parse.o
	./hoc.sh src/node.c
	$(CC) -g -c src/node.s -o gen_first/node.o
	./hoc.sh src/emit.c
	$(CC) -g -c src/emit.s -o gen_first/emit.o
	./hoc.sh src/utils.c
	$(CC) -g -c src/utils.s -o gen_first/utils.o
	./hoc.sh src/cpp.c
	$(CC) -g -c src/cpp.s -o gen_first/cpp.o
	$(CC) -g -static -o gen_first/hoc $(OBJS:src/%=gen_first/%) $(LDFLAGS)

gen_second: selfhost FORCE
	./hoc_1.sh src/containers.c
	$(CC) -g -c src/containers_1.s -o gen_second/containers.o
	./hoc_1.sh src/main.c
	$(CC) -g -c src/main_1.s -o gen_second/main.o
	./hoc_1.sh src/token.c
	$(CC) -c src/token_1.s -o gen_second/token.o
	./hoc_1.sh src/sema.c
	$(CC) -g -c src/sema_1.s -o gen_second/sema.o
	./hoc_1.sh src/parse.c
	$(CC) -c src/parse_1.s -o gen_second/parse.o
	./hoc_1.sh src/node.c
	$(CC) -g -c src/node_1.s -o gen_second/node.o
	./hoc_1.sh src/emit.c
	$(CC) -g -c src/emit_1.s -o gen_second/emit.o
	./hoc_1.sh src/utils.c
	$(CC) -g -c src/utils_1.s -o gen_second/utils.o
	./hoc_1.sh src/cpp.c
	$(CC) -g -c src/cpp_1.s -o gen_second/cpp.o
	$(CC) -g -static -o gen_second/hoc $(OBJS:src/%=gen_second/%) $(LDFLAGS)

test: hoc selfhost gen_second FORCE
	./test.sh
	./hoc test/pp_test.c > test/pp_test.s
	gcc -g -static -o test/pp_test test/pp_test.s
	./test/pp_test

clean:
	$(RM) hoc $(OBJS) $(OBJS:src/%=gen_first/%) $(SRCS:%.c=%.s)
	$(RM) src/*_pp.c src/*_pp.o src/*_1.s test/pp_test

FORCE:
.PHONY: clean FORCE
