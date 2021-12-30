//
// Created by rafael on 28/10/21.
//
#include "cliente.h"

#define FIFO_SERV "canal"
#define FIFO_CLI "cli%d"
#define FIFO_LIG "lig%d"


int main(int argc,char *argv[])
{
    cliente a;
    char sintoma[100], str[40], str_lig[40], str_com[40];
    int fd, n, fd_cli = 0, n_cli, fd_lig = 0, res_com, maxfd;
    pedido p;
    struct timeval tempo;
    fd_set fds;

    if(argc < 2)
    {
        printf("\n<ERRO> Indique o seu nome como parametro");
        return 1;
    }

    strcpy(a.nome, argv[1]);


    p.pid_cli = getpid();

    if(access(FIFO_SERV, F_OK) != 0){
        printf("\nO servidor esta desligado...\n");
        exit(1);
    }


    sprintf(str, FIFO_CLI, p.pid_cli);
    mkfifo(str, 0600);


    printf("\n[PID=%d]Bom dia %s",p.pid_cli, a.nome);

    p.cli_med = 0;
    p.pid_med = 0;

    printf("\nQuais sao os seus sistomas?\n");
    fgets(p.sintomas, sizeof(p.sintomas), stdin);


    fd = open(FIFO_SERV, O_WRONLY);
    if(fd == -1){
        printf("\nNao conseguiu abrir o FIFo do balcao...");
        exit(1);
    }
    n = write(fd, &p, sizeof(pedido));
    if(n == -1){
        printf("\nNao conseguiu escrever para o balcao...");
        exit(1);
    }
    close(fd);


    fd_cli = open(str, O_RDWR);
    read(fd_cli, &p, sizeof(pedido));
    printf("\nO seu diagnostico e: %s", p.classificacao);
    close(fd_cli);


    fd_cli = open(str, O_RDWR);
    if(fd_cli == -1){
        printf("\nNao conseguiu abrir o FIFO do cliente...");
        exit(1);
    }
    do{
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_cli, &fds);
        FD_SET(fd_lig, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        res_com = select(fd_cli+1, &fds, NULL, NULL, &tempo);
        if(res_com == 0){
            printf("\nA espera...");
        }else if(res_com > 0){
            if(FD_ISSET(0, &fds)){
                fgets(str_com, sizeof(str_com), stdin);
                break;
            }
            if(FD_ISSET(fd_cli, &fds)){

                read(fd_cli, &p, sizeof(pedido));

                printf("\npid_med: %d \t pid_cli: %d", p.pid_med, p.pid_cli);

                sprintf(str_lig, FIFO_LIG, p.pid_med);
                if(access(str_lig, F_OK) == 0){

                    printf("\nEsta conectado ao seu medico!\nR: ");
                    fgets(p.msg, sizeof(p.msg), stdin);

                    fd_lig = open(str_lig, O_RDWR);
                    write(fd_lig, &p, sizeof(pedido));

                }
            }
            if(FD_ISSET(fd_lig, &fds)){
                read(fd_lig, &p, sizeof(pedido));

                printf("\n%s\nR: ", p.msg);

                if(strcmp(p.msg, "acabou\n") != 0){
                    fgets(p.msg, sizeof(p.msg), stdin);

                    write(fd_lig, &p, sizeof(pedido));
                }
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);


    close(fd_lig);
    close(fd_cli);
    unlink(str);

    return 0;
}



