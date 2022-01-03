//
// Created by rafael on 28/10/21.
//
#include <stdio.h>
#include "balcao.h"

#define FIFO_SERV "canal"
#define FIFO_MED "med%d"
#define FIFO_LIG_M "ligM%d"
#define FIFO_LIG_C "ligC%d"


int main(int argc, char* argv[])
{
    int fd, n, fd_med = 0, n_med, res_com, fd_lig_c = 0, fd_lig_m = 0, maxfd;
    pedido p;
    char str[40], str_lig_c[40], str_lig_m[40], str_com[40];
    struct timeval tempo;
    fd_set fds;


    if(argc < 3){
        printf("\nIndique o seu nome e a sua especialidade por parametro");
        return 1;
    }


    p.pid_med = getpid();
    printf("\n[PID:%d]Ola %s da especialidade %s\n",p.pid_med, argv[1], argv[2]);


    if(access(FIFO_SERV, F_OK) != 0){
        printf("\nO servidor esta desligado...\n");
        exit(1);
    }

    sprintf(str, FIFO_MED, p.pid_med);
    mkfifo(str, 0600);


    sprintf(str_lig_m, FIFO_LIG_M, p.pid_med);
    mkfifo(str_lig_m, 0600);


    p.cli_med = 1;
    strcpy(p.especialidade, argv[2]);

    fd = open(FIFO_SERV, O_WRONLY);
    if(fd == -1){
        printf("\nNao conseguiu abrir o FIFo do balcao...");
        exit(1);
    }
    write(fd, &p, sizeof(pedido));
    close(fd);



    fd_lig_m = open(str_lig_m, O_RDWR | O_NONBLOCK);
    if(fd_lig_m == -1){
        perror("\nImpossivel abrir o FIFO da ligacao");
        exit(1);
    }
    fd_med = open(str, O_RDWR | O_NONBLOCK);
    if(fd_med == -1){
        perror("\nImpossivel abrir o FIFO do medico");
        exit(1);
    }

    do {
        printf("\n");


        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_med, &fds);
        FD_SET(fd_lig_m, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        maxfd = fd_med > fd_lig_m ? fd_med : fd_lig_m;

        res_com = select(maxfd+1, &fds, NULL, NULL, &tempo);

        if(res_com == 0){
            printf("\nA espera de cliente...");
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
                    strcpy(str_com, "sair\n");
                }
            }
            if(FD_ISSET(fd_lig_m, &fds)){
                n = read(fd_lig_m, &p, sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler do cliente...");
                    exit(1);
                }

                printf("\n[PID_CLIENTE: %d] %s\nR: ", p.pid_cli, p.msg);

                fgets(p.msg, sizeof(p.msg), stdin);
                if(strcmp(p.msg, "sair\n") == 0){
                    strcpy(str_com, p.msg);
                    strcpy(p.msg, "acabou\n");
                }

                sprintf(str_lig_c, FIFO_LIG_C, p.pid_cli);
                fd_lig_c = open(str_lig_c, O_WRONLY);
                n = write(fd_lig_c, &p, sizeof(pedido));
                if(n == -1){
                    printf("\nNao conseguiu escrever para o cliente...");
                    exit(1);
                }
                close(fd_lig_c);
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);




    close(fd_med);
    close(fd_lig_m);

    unlink(str);
    unlink(str_lig_m);

    exit(0);
}
