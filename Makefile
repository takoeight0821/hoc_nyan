CFLAGS=-g -Wall -std=c11 -I./include
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): include/hoc.h

selfhost:
	./hoc src/containers.c > src/containers.s
	$(CC) -g -c src/containers.s -o gen_first/containers.o
	./hoc src/main.c > src/main.s
	$(CC) -g -c src/main.s -o gen_first/main.o
	./hoc src/token.c > src/token.s
	$(CC) -c src/token.s -o gen_first/token.o
	./hoc src/sema.c > src/sema.s
	$(CC) -g -c src/sema.s -o gen_first/sema.o
	./hoc src/parse.c > src/parse.s
	$(CC) -c src/parse.s -o gen_first/parse.o
	./hoc src/node.c > src/node.s
	$(CC) -g -c src/node.s -o gen_first/node.o
	./hoc src/emit.c > src/emit.s
	$(CC) -g -c src/emit.s -o gen_first/emit.o
	./hoc src/utils.c > src/utils.s
	$(CC) -g -c src/utils.s -o gen_first/utils.o
	./hoc src/cpp.c > src/cpp.s
	$(CC) -g -c src/cpp.s -o gen_first/cpp.o
	$(CC) -g -static -o gen_first/hoc $(OBJS:src/%=gen_first/%) $(LDFLAGS)

gen_second: selfhost FORCE
	./gen_first/hoc src/containers.c > src/containers_1.s
	$(CC) -g -c src/containers_1.s -o gen_second/containers.o
	./gen_first/hoc src/main.c > src/main_1.s
	$(CC) -g -c src/main_1.s -o gen_second/main.o
	./gen_first/hoc src/token.c > src/token_1.s
	$(CC) -c src/token_1.s -o gen_second/token.o
	./gen_first/hoc src/sema.c > src/sema_1.s
	$(CC) -g -c src/sema_1.s -o gen_second/sema.o
	./gen_first/hoc src/parse.c > src/parse_1.s
	$(CC) -c src/parse_1.s -o gen_second/parse.o
	./gen_first/hoc src/node.c > src/node_1.s
	$(CC) -g -c src/node_1.s -o gen_second/node.o
	./gen_first/hoc src/emit.c > src/emit_1.s
	$(CC) -g -c src/emit_1.s -o gen_second/emit.o
	./gen_first/hoc src/utils.c > src/utils_1.s
	$(CC) -g -c src/utils_1.s -o gen_second/utils.o
	./gen_first/hoc src/cpp.c > src/cpp_1.s
	$(CC) -g -c src/cpp_1.s -o gen_second/cpp.o
	$(CC) -g -static -o gen_second/hoc $(OBJS:src/%=gen_second/%) $(LDFLAGS)

test: hoc selfhost gen_second FORCE
	./test.sh
	./hoc test/pp_test.c > test/pp_test.s
	gcc -g -static -o test/pp_test.out test/pp_test.s
	./test/pp_test.out

clean:
	$(RM) hoc $(OBJS) $(OBJS:src/%=gen_first/%) $(SRCS:%.c=%.s)
	$(RM) src/*_pp.c src/*_pp.o src/*_1.s test/pp_test.out

FORCE:
.PHONY: clean FORCE
