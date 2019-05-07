CC := gcc

main: main.c hoc.h vec.c node.c utils.c emit.c
	cc main.c vec.c node.c utils.c emit.c -o main

clean:
	$(RM) main
