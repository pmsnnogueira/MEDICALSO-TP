#ifndef TP_SO_BIBLIO_H
#define TP_SO_BIBLIO_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>


typedef struct{
    int pid_cli, pid_med, cli_med;//se o cli_med estiver a 0 é um cliente, se estiver a 1 é um médico
    char sintomas[40], classificacao[40], msg[100], especialidade[40];
} pedido;

#endif //TP_SO_BIBLIO_H
