all: balcao.o cliente.o medico.o
	gcc balcao.o -o balcao -lpthread
	gcc cliente.o -o cliente -lpthread
	gcc medico.o -o medico -lpthread

balcao.o: balcao.c
	gcc -c balcao.c

cliente.o: cliente.c
	gcc -c cliente.c

medico.o: medico.c
	gcc -c medico.c

limpa:
	rm -f *.o
	rm -f canal
	rm -f lig*
	rm -f Fcli*
	rm -f Fmed*
	rm -f sinal
