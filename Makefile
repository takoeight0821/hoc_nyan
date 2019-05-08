CC := gcc
CFLAGS = -Wall

main: main.c hoc.h vec.c node.c utils.c emit.c lex.c parse.c
	$(CC) $(CFLAGS) main.c vec.c node.c utils.c emit.c lex.c parse.c -o main

clean:
	$(RM) main
