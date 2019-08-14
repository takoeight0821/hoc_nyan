CFLAGS=-g -Wall -std=c11
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

SH_SRCS=src/containers.c

hoc: $(OBJS)
	$(CC) -static -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): src/hoc.h

prepare_selfhost: hoc FORCE
	./hoc.sh src/containers.c
	$(CC) -g -c src/containers.s -o gen_first/containers.o
	./hoc.sh src/main.c
	$(CC) -g -c src/main.s -o gen_first/main.o
# undefined function
#	./hoc.sh src/token.c
#	$(CC) -c src/token.s -o gen_first/token.o
	cp src/token.o gen_first/token.o
	./hoc.sh src/sema.c
	$(CC) -g -c src/sema.s -o gen_first/sema.o
# Error: invalid use of operator "lt"
#	./hoc.sh src/parse.c
#	$(CC) -c src/parse.s -o gen_first/parse.o
	cp src/parse.o gen_first/parse.o
	./hoc.sh src/node.c
	$(CC) -g -c src/node.s -o gen_first/node.o
	./hoc.sh src/emit.c
	$(CC) -g -c src/emit.s -o gen_first/emit.o
#	cp src/utils.o gen_first/utils.o
	./hoc.sh src/utils.c
	$(CC) -g -c src/utils.s -o gen_first/utils.o

selfhost: prepare_selfhost
	$(CC) -static -o gen_first/hoc $(OBJS:src/%=gen_first/%) $(LDFLAGS)

test: hoc selfhost FORCE
	./test.sh

clean:
	$(RM) hoc $(OBJS) $(OBJS:src/%=gen_first/%) $(SRCS:%.c=%.s)
	$(RM) src/*_pp.c src/*_pp.o

FORCE:
.PHONY: clean FORCE
