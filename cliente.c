//
// Created by rafael on 28/10/21.
//
#include <stdio.h>
#include "cliente.h"
#include <unistd.h>


int main(int argc,char *argv[])
{
    cliente a;
    char sintoma[100];
    int pid = getpid();


    if(argc < 2)
    {
        printf("\n<ERRO> Indique o seu nome como parametro");
        return 1;
    }

    a.nome = argv[1];

    printf("\n[PID=%d]Bom dia %s",pid, a.nome);

    printf("\nQuais sao os seus sistomas?\n");
    fgets(sintoma, sizeof(sintoma), stdin);

    a.sintomas = sintoma;
    printf("\n%s", a.sintomas);


    return 0;
}

