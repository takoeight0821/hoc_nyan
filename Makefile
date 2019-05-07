CC := gcc

main: main.c hoc.h vec.c
	cc main.c vec.c -o main

clean:
	$(RM) main
