CFLAGS=-Wall -Wextra -std=gnu11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

hoc: $(OBJS)
	$(CC) -o hoc $(OBJS) $(LDFLAGS)

$(OBJS): hoc.h

test: hoc
	./hoc -test
	./test.sh

clean:
	$(RM) hoc $(OBJS)
