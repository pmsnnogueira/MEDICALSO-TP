#ifndef TP_SO_BIBLIO_H
#define TP_SO_BIBLIO_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define FIFO_SERV "canal"
#define FIFO_CLI "Lcli%d"
#define FIFO_MED "Lmed%d"
#define FIFO_SINAL "sinal"


typedef struct{
    int pid_cli, pid_med, cli_med, com, sair, temp, prio;//se o cli_med estiver a 0 é um cliente, se estiver a 1 é um médico
    char sintomas[40], classificacao[40], msg[100], especialidade[40];
} pedido;

#endif //TP_SO_BIBLIO_H
