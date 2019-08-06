CFLAGS=-g -Wall -std=c11
SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): src/hoc.h

test: hoc FORCE
	./test.sh

clean:
	$(RM) hoc $(OBJS)

FORCE:
.PHONY: clean FORCE
