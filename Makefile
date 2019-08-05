CFLAGS=-g -Wall -std=c11
SRCS=containers.c main.c utils.c token.c parse.c node.c sema.c emit.c
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): hoc.h

test: hoc FORCE
	./hoc -test
	./test.sh

clean:
	$(RM) hoc $(OBJS)

FORCE:
.PHONY: clean FORCE
