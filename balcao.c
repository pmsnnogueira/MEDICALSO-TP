//
// Created by rafael on 28/10/21.
//

#include "balcao.h"

#define FIFO_SERV "canal"
#define FIFO_CLI "cli%d"
#define FIFO_MED "med%d"


int comandos(char *frase){
	int tam = strlen(frase)-1;
    const char comandos[7][25] = {
              "utentes",
              "especialistas",
              "delut",
              "delesp",
              "freq",
              "encerra"};

     for(int i=0 ; i < 7 ; i++)
     {
		//printf("%ld-",strlen(frase));
         if(strncmp(frase,comandos[0],tam) == 0)
         {
              printf("Comando Utentes\n");
              break;
         }
         if(strncmp(frase,comandos[1],tam) == 0)
         {
             printf("especialistas\n");
             break;
         }
         if(strncmp(frase,comandos[2],tam) == 0)
         {
             printf("delut\n");
             break;
         }
         if(strncmp(frase,comandos[3],tam) == 0)
         {
             printf("delesp\n");
             break;
         }
         if(strncmp(frase,comandos[4],tam) == 0)
         {
             printf("freq\n");


             break;
         }
         if(strncmp(frase,comandos[5],tam) == 0)
         {
             printf("Comando encerra\n");

             return 2;
         }

     }

	return 0;
}


int main(int argc, char* argv[], char* envp[]) {
    int i, canal[2], retorno[2], res, res_fork, res_pipe, estado, var, n_write;
    int n_fifo, fd, fd_cli, res_com, fd_med;
    char str[100], str1[40], str_cli[40], str_com[40], str_med[40];
    balcao b;
    struct timeval tempo;
    fd_set fds;

    pedido p;

    setbuf(stdout, NULL);

    if (getenv("MAXCLIENTES") == NULL) {
        printf("Erro nas variaveis de Ambiente 'MAXCLIENTES'\n");

    } else {
        var = atoi(getenv("MAXCLIENTES"));
        if (var > 0) {
            b.maxclientes = var;
            printf("O valor de maxclientes e: %d", b.maxclientes);
        } else {
            printf("O valor tem de ser superior a 0 \n");
        }

    }

    if (getenv("MAXMEDICOS") == NULL) {
        printf("Erro nas variaveis de ambiente 'MAXMEDICOS'\n");

    } else {
        var = atoi(getenv("MAXMEDICOS"));
        if (var > 0) {
            b.maxmedicos = var;
            printf("\nO valor de maxmedicos e: %d\n", b.maxmedicos);
        } else {
            printf("O valor tem de ser superior a 0 \n");
        }
    }

    pedido p_cli[b.maxclientes], p_med[b.maxmedicos];//estruturas para armazenar clientes e medicos
    int ite_cli = 0, ite_med = 0;//"iteradores"


    if (access(FIFO_SERV, F_OK) == 0) {
        printf("[ERRO] O FIFO já existe\n");
        exit(1);
    }
    mkfifo(FIFO_SERV, 0600);

    fd = open(FIFO_SERV, O_RDWR);
    if (fd == -1) {
        perror("\nImpossivel abrir o FIFO");
        exit(1);
    }

    res_pipe = pipe(canal);
    if (res_pipe < 0) {
        perror("\nerro pipe1: ");
        return 1;
    }

    res_pipe = pipe(retorno);
    if (res_pipe < 0) {
        perror("\nerro pipe2: ");
        return 1;
    }

    res_fork = fork();
    if (res_fork < 0) {
        perror("\nerro fork: ");
        return 1;
    }
    if (res_fork == 0) {//FILHO
        close(0);
        close(1);
        dup(canal[0]);
        dup(retorno[1]);
        close(canal[0]);
        close(retorno[0]);
        close(canal[1]);
        close(retorno[1]);
        execl("classificador", "classificador", NULL);
        fprintf(stderr, "\no filho deu erro: %s", str1);
        exit(123);
    }
    //PAI
    close(canal[0]);
    close(retorno[1]);


    do {
        printf("\nComandos/Sintomas: ");

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd, &fds);

        tempo.tv_sec = 10;
        tempo.tv_usec = 0;
        res_com = select(fd + 1, &fds, NULL, NULL, &tempo);
        if (res_com == 0) {
            printf("\nSem input...\n");
        } else if (res_com > 0) {
            if (FD_ISSET(0, &fds)) {
                fgets(str_com, sizeof(str_com), stdin);
                if (comandos(str_com) == 2) {
                    printf("\nA terminar o programa...");
                    break;
                }
            }
            if (FD_ISSET(fd, &fds)) {

                n_fifo = read(fd, &p, sizeof(pedido));
                if (n_fifo == -1) {
                    printf("\nNao conseguiu ler...");
                    exit(1);
                }
                if (n_fifo == sizeof(pedido)) {
                    if (p.cli_med == 0) {
                        if (ite_cli <= b.maxclientes - 1) {
                            n_write = write(canal[1], p.sintomas, strlen(p.sintomas));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            res = read(retorno[0], p.classificacao, sizeof(p.classificacao) - 1);
                            if (res == -1) {
                                printf("\nNão conseguiu ler...");
                                exit(1);
                            }
                            p.classificacao[res] = '\0';

                            sprintf(str_cli, FIFO_CLI, p.pid_cli);
                            fd_cli = open(str_cli, O_WRONLY);
                            n_write = write(fd_cli, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_cli);
                            p_cli[ite_cli] = p;
                            ++ite_cli;
                            printf("\nN. clientes: %d", ite_cli);
                            printf("\nSintomas: %s", p_cli[ite_cli-1].sintomas);
                        } else {
                            strcpy(p.classificacao, "O balcao nao consegue atender mais clientes...");
                            sprintf(str_cli, FIFO_CLI, p.pid_cli);
                            fd_cli = open(str_cli, O_WRONLY);
                            n_write = write(fd_cli, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_cli);
                            continue;
                        }
                    } else {
                        if (ite_med <= b.maxmedicos - 1) {
                            strcpy(p.msg, "esta ligado ao balcao!");
                            sprintf(str_med, FIFO_MED, p.pid_med);
                            fd_med = open(str_med, O_WRONLY);
                            n_write = write(fd_med, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_med);
                            p_med[ite_med] = p;
                            ++ite_med;
                            printf("\nN. medicos: %d", ite_med);
                        } else {
                            strcpy(p.msg, "O balcao nao tem capacidade para mais medicos...");
                            sprintf(str_med, FIFO_MED, p.pid_med);
                            fd_med = open(str_med, O_WRONLY);
                            n_write = write(fd_med, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_med);
                            continue;
                        }
                    }
                }
            }
        }

        if(ite_cli != 0 && ite_med != 0){
            p_cli[0].pid_med = p_med[0].pid_med;
            printf("\ncli: %d\tmed: %d\tmed_passado: %d", p_cli[0].pid_cli, p_med[0].pid_med, p_cli[0].pid_med);

            fd_cli = open(str_cli, O_WRONLY);
            n_write = write(fd_cli, &p_cli[0], sizeof(pedido));
            if(n_write == -1){
                printf("\nNao conseguiu mandar a informação para o cliente...");
                close(fd_cli);
                break;
            }
            close(fd_cli);
        }
    }while(strcmp(str_com, "sair\n") != 0);



    close(fd);
    unlink(FIFO_SERV);

    return 0;
}

