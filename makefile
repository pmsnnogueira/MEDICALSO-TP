all: balcao.o
	gcc balcao.o -o balcao

balcao.o: balcao.c
	gcc -c balcao.c

limpa:
	rm -f *.o

