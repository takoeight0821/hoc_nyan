CC := gcc

main: main.c hoc.h vec.c node.c
	cc main.c vec.c node.c -o main

clean:
	$(RM) main
