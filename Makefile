CC := gcc

main: main.c
	cc main.c -o main

clean:
	$(RM) main
