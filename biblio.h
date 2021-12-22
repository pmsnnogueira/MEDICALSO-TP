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
    int pid;
    char sintomas[40], classificacao[40];
} pedido;

#endif //TP_SO_BIBLIO_H
