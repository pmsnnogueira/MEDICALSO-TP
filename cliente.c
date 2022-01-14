//
// Created by rafael on 28/10/21.
//


#include "biblio.h"

int main(int argc,char *argv[])
{
    char nome[100], *ptr;
    char sintoma[100], str[40], str_lig_m[40], str_com[40], str_lig_c[40];
    int fd, n, fd_cli = 0, n_cli, fd_lig_c = 0, res_com, fd_lig_m = 0, maxfd, estado;
    pedido p;
    struct timeval tempo;
    fd_set fds;
    pthread_t tid;

    if(argc < 2)
    {
        printf("\n<ERRO> Indique o seu nome como parametro");
        return 1;
    }


    setbuf(stdout, NULL);


    strcpy(nome, argv[1]);


    p.pid_cli = getpid();

    if(access(FIFO_SERV, F_OK) != 0){
        printf("\nO servidor esta desligado...\n");
        exit(1);
    }


    sprintf(str, FIFO_CLI, p.pid_cli);
    mkfifo(str, 0600);


    printf("\n[PID=%d]Bom dia %s",p.pid_cli, nome);

    p.cli_med = 0;
    p.pid_med = 0;
    p.com = 0;
    p.sair = 0;

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


    fd_cli = open(str, O_RDONLY);
    n = read(fd_cli, &p, sizeof(pedido));
    if(n == -1){
        printf("\nNao conseguiu ler do balcao...");
        exit(1);
    }
    close(fd_cli);

    ptr = strtok(p.classificacao, " ");
    strcpy(sintoma, ptr);
    ptr = strtok(NULL, " ");
    strcpy(p.classificacao, sintoma);
    p.prio = atoi(ptr);

    estado = 1;

    printf("\nO seu diagnostico e: %s e tem prioridade: ", p.classificacao);
    fflush(stdout);


    fd_cli = open(str, O_RDWR | O_NONBLOCK);
    if(fd_cli == -1){
        printf("\nNao conseguiu abrir o FIFO do cliente...");
        exit(1);
    }

    do{
        if(estado == 2){
            printf("R: ");
        }

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_cli, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        res_com = select(fd_cli+1, &fds, NULL, NULL, &tempo);

        if(res_com == 0){
            printf("\nA espera...");
        }else if(res_com > 0){
            if(FD_ISSET(0, &fds)){
                fgets(str_com, sizeof(str_com), stdin);

                strcpy(p.msg, str_com);

                if(estado == 1){
                    //se ja tiver recebido o confirmação do balcao, tudo o que for escrito é enviado para po balcao
                    fd = open(FIFO_SERV, O_WRONLY);
                    if(fd == -1){
                        printf("\nNao conseguiu abrir o FIFo do balcao...");
                        exit(1);
                    }

                    p.cli_med = 0;

                    n = write(fd, &p, sizeof(pedido));
                    if(n == -1){
                        printf("\nNao conseguiu escrever para o balcao...");
                        exit(1);
                    }
                    close(fd);
                }else{
                    //se estiver na consulta tudo que escrever vai ser enviado para o medico
                    sprintf(str_lig_m, FIFO_MED, p.pid_med);

                    p.cli_med = 0;

                    if(access(str_lig_m, F_OK) == 0){
                        fd_lig_m = open(str_lig_m, O_WRONLY);
                        write(fd_lig_m, &p, sizeof(pedido));
                        close(fd_lig_m);
                    }
                }
            }
            if(FD_ISSET(fd_cli, &fds)){
                n = read(fd_cli, &p, sizeof(pedido));
                if(n == -1){
                    printf("\nNao conseguiu ler do balcao...");
                    exit(1);
                }

                if(p.cli_med == 1){
                    estado = 2;

                    printf("\n[PID_MED: %d]Enviou: %s", p.pid_med, p.msg);
                    if(strcmp(p.msg, "acabou\n") == 0){
                        estado = 1;
                        printf("\nA consulta acabou...\n");
                        strcpy(str_com, "sair\n");
                    }

                }else{
                    estado = 1;
                }
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);


    //quando o cliente encerra da sinal ao balcao
    p.sair = 1;
    p.cli_med = 0;

    fd = open(FIFO_SINAL, O_WRONLY);
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

    strcpy(str_com, "sair\n");

    close(fd_lig_c);
    close(fd_cli);
    unlink(str);
    unlink(str_lig_c);

    exit(0);
}

