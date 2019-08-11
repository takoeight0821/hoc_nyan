CFLAGS=-g -Wall -std=c11
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

SH_SRCS=src/containers.c

hoc: $(OBJS)
	$(CC) -static -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): src/hoc.h

prepare_self_host: hoc FORCE
	./hoc.sh src/containers.c
	$(CC) -c src/containers.s -o src/containers.o
	./hoc.sh src/main.c
	$(CC) -c src/main.s -o src/main.o
# undefined function
#	./hoc.sh src/token.c
#	$(CC) -c src/token.s -o src/token.o

# segmentation fault
#	./hoc.sh src/sema.c
#	$(CC) -c src/sema.s -o src/sema.o
#	./hoc.sh src/parse.c
#	$(CC) -c src/parse.s -o src/parse.o
# ./hoc.sh src/node.c
# $(CC) -c src/node.s -o src/node.o

test: hoc FORCE
	./test.sh

clean:
	$(RM) hoc $(OBJS) src/containers.s src/main.s

FORCE:
.PHONY: clean FORCE
