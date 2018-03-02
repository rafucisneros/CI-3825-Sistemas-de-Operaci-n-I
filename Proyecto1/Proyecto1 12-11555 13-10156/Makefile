HEADERS = palindromo.h parser.h
default: main

main.o: main.c $(HEADERS)
	gcc -c main.c -o main.o

main: main.o
	gcc -pthread main.o -o main

clean:
	-rm -f main.o
	-rm -f main