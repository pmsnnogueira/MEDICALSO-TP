//
// Created by rafael on 28/10/21.
//
#include <stdio.h>
#include "balcao.h"

#define FIFO_SERV "canal"
#define FIFO_MED "med%d"
#define FIFO_LIG "lig%d"


int main(int argc, char* argv[])
{
    int fd, n, fd_med = 0, n_med, res_com, fd_lig = 0;
    pedido p;
    char str[40], str_lig[40], str_com[40];
    struct timeval tempo;
    fd_set fds;


    if(argc < 3){
        printf("\nIndique o seu nome e a sua especialidade por parametro");
        return 1;
    }

    printf("\nOla %s da especialidade %s\n", argv[1], argv[2]);


    p.pid_med = getpid();

    if(access(FIFO_SERV, F_OK) != 0){
        printf("\nO servidor esta desligado...\n");
        exit(1);
    }

    sprintf(str, FIFO_MED, p.pid_med);
    mkfifo(str, 0600);
    fd_med = open(str, O_RDWR);
    if(fd_med == -1){
        perror("\nImpossivel abrir o FIFO do medico");
        exit(1);
    }

    sprintf(str_lig, FIFO_LIG, p.pid_med);
    mkfifo(str_lig, 0600);
    fd_lig = open(str_lig, O_RDWR);
    if(fd_lig == -1){
        perror("\nImpossivel abrir o FIFO da ligacao");
        exit(1);
    }


    p.cli_med = 1;
    strcpy(p.especialidade, argv[2]);

    fd = open(FIFO_SERV, O_WRONLY);
    if(fd == -1){
        printf("\nNao conseguiu abrir o FIFo do balcao...");
        exit(1);
    }
    write(fd, &p, sizeof(pedido));
    close(fd);

    do {
        printf("\n");


        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_med, &fds);
        FD_SET(fd_lig, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;
        res_com = select(fd_lig+1, &fds, NULL, NULL, &tempo);

        if(res_com == 0){
            printf("\nA esspera de cliente...");
        }else if(res_com > 0){
            if(FD_ISSET(0, &fds)){
                fgets(str_com, sizeof(str_com), stdin);
                break;
            }
            if(FD_ISSET(fd_med, &fds)){
                n = read(fd_med, &p, sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler...");
                    exit(1);
                }

                if (strcmp(p.msg, "O balcao nao tem capacidade para mais medicos...") == 0) {
                    printf("%s", p.msg);
                    strcpy(str_com, "sair");
                }
            }
            if(FD_ISSET(fd_lig, &fds)){
                n = read(fd_lig, &p, sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler do cliente...");
                    exit(1);
                }

                printf("\n%s\nR: ", p.msg);

                fgets(p.msg, sizeof(p.msg), stdin);

                n = write(fd_lig, &p, sizeof(pedido));
                if(n == -1){
                    printf("\nNao conseguiu escrever para o cliente...");
                    exit(1);
                }
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);




    close(fd_med);
    close(fd_lig);

    unlink(str);
    unlink(str_lig);

    exit(0);
}
