CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): hoc.h

test: hoc
	./test.sh

clean:
	$(RM) hoc $(OBJS)
