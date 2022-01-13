//
// Created by rafael on 28/10/21.
//
#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>


#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#define FIFO_SERV "canal"
#define FIFO_MED "med%d"
#define FIFO_SINAL "sinal"
#define FIFO_CLI "cli%d"


typedef struct{
    int pid_cli, pid_med, cli_med, com, sair, temp, prio;//se o cli_med estiver a 0 é um cliente, se estiver a 1 é um médico
    char sintomas[40], classificacao[40], msg[100], especialidade[40];
} pedido;



void acorda(int s, siginfo_t *info, void* uc){}


void *temporizador(void* dados){
    char cmd[100];
    int fd;

    pedido *td = (pedido *) dados;
    do{
        sleep(20);

        fd = open(FIFO_SINAL, O_WRONLY);

        td->cli_med = 1;

        write(fd, td, sizeof(pedido));

        close(fd);

    }while(td->sair == 0);
    pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
    int fd, n, fd_med = 0, n_med, res_com, fd_lig_m = 0, fd_lig_c = 0, maxfd, estado = 0;
    pedido p;
    char str[40], str_lig_m[40], str_com[40], str_lig_c[40];
    struct timeval tempo;
    fd_set fds;
    pthread_t tid;

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



    p.cli_med = 1;
    p.com = 0;
    p.sair = 0;
    strcpy(p.especialidade, argv[2]);

    fd = open(FIFO_SERV, O_WRONLY);
    if(fd == -1){
        printf("\nNao conseguiu abrir o FIFo do balcao...");
        exit(1);
    }
    write(fd, &p, sizeof(pedido));
    close(fd);


    pthread_create(&tid, NULL, temporizador, (void* ) &p);


    fd_med = open(str, O_RDWR | O_NONBLOCK);
    if(fd_med == -1){
        perror("\nImpossivel abrir o FIFO do medico");
        exit(1);
    }

    do {
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_med, &fds);
        FD_SET(fd_lig_m, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        maxfd = fd_med > fd_lig_m ? fd_med : fd_lig_m;

        res_com = select(maxfd+1, &fds, NULL, NULL, &tempo);

        if(res_com == 0){
            printf("\nA espera de clientes...");
        }else if(res_com > 0){
            if(FD_ISSET(0, &fds)){
                fgets(str_com, sizeof(str_com), stdin);

                strcpy(p.msg, str_com);

                if(estado == 1){
                    fd = open(FIFO_SERV, O_WRONLY);
                    if(fd == -1){
                        printf("\nNao conseguiu abrir o FIFo do balcao...");
                        exit(1);
                    }

                    p.cli_med = 1;
                    write(fd, &p, sizeof(pedido));
                    close(fd);
                }else{
                    sprintf(str_lig_c, FIFO_CLI, p.pid_cli);

                    p.cli_med = 1;

                    if(access(str_lig_c, F_OK) == 0){
                        fd_lig_c = open(str_lig_c, O_WRONLY);
                        write(fd_lig_c, &p, sizeof(pedido));
                        close(fd_lig_c);
                    }
                }
            }
            if(FD_ISSET(fd_med, &fds)){
                n = read(fd_med, &p, sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler...");
                    exit(1);
                }

                if(p.cli_med == 0){
                    estado = 2;

                    printf("\n[PID_CLI: %d]Enviou: %s", p.pid_cli, p.msg);
                }else{
                    estado = 1;

                    if (strcmp(p.msg, "O balcao nao tem capacidade para mais medicos...") == 0) {
                        printf("%s", p.msg);
                        strcpy(str_com, "sair\n");
                    }
                    if (strcmp(p.msg, "o balcao fechou") == 0) {
                        printf("%s", p.msg);
                        strcpy(str_com, "sair\n");
                    }
                }
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);



    close(fd_med);
    close(fd_lig_m);

    p.sair = 1;
    pthread_join(tid, NULL);

    unlink(str);
    unlink(str_lig_m);

    exit(0);
}
