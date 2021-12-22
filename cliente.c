//
// Created by rafael on 28/10/21.
//
#include "cliente.h"

#define FIFO_SERV "canal"
#define FIFO_CLI "cli%d"


int main(int argc,char *argv[])
{
    cliente a;
    char sintoma[100], str[40];
    int fd, n, fd_cli, n_cli;
    pedido  p;


    if(argc < 2)
    {
        printf("\n<ERRO> Indique o seu nome como parametro");
        return 1;
    }

    strcpy(a.nome, argv[1]);

    p.pid = getpid();

    sprintf(str, FIFO_CLI, p.pid);
    mkfifo(str, 0600);

    if(access(FIFO_SERV, F_OK) != 0){
        printf("\nO servidor está desligado...");
        exit(1);
    }

    fd = open(FIFO_SERV, O_WRONLY);

    printf("\n[PID=%d]Bem-Vindo, %s", p.pid, a.nome);
    do{
        printf("\nQuais sao os seus sistomas?\n");
        fgets(p.sintomas, sizeof(p.sintomas), stdin);

        write(fd, &p, sizeof(pedido));

        fd_cli = open(str, O_RDONLY);
        read(fd_cli, &p, sizeof(pedido));
        close(fd_cli);
        printf("\nO seu diagnostico e: %s", p.classificacao);
    }while(strcmp(p.sintomas, "sair\n"));

    strcpy(a.sintomas, p.sintomas);

    close(fd_cli);
    unlink(str);

    return 0;
}

