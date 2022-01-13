
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define FIFO_SERV "canal"
#define FIFO_CLI "cli%d"
#define FIFO_MED "med%d"
#define FIFO_SINAL "sinal"

typedef struct{
    int pid_cli, pid_med, cli_med, com, sair, temp, prio;//se o cli_med estiver a 0 é um cliente, se estiver a 1 é um médico
    char sintomas[40], classificacao[40], msg[100], especialidade[40];
} pedido;


typedef struct Balcao{
    int continua;
    int maxclientes, maxmedicos;
    pedido p_cli[5];
    pedido p_med[5];
    int ite_cli , ite_med;//"iteradores"

    int tempo; //Tempo para mostrar a lista

}balcao;

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
        if(strncmp(frase,comandos[0],tam) == 0)
        {
            printf("Comando Utentes\n");
            return 1;
        }
        if(strncmp(frase,comandos[1],tam) == 0)
        {
            printf("especialistas\n");
            return 2;
        }
        if(strncmp(frase,comandos[2],tam) == 0)
        {
            printf("delut\n");
            return 3;
        }
        if(strncmp(frase,comandos[3],tam) == 0)
        {
            printf("delesp\n");
            return 4;
        }
        if(strncmp(frase,comandos[4],tam) == 0)
        {
            printf("freq\n");
            return 5;

        }
        if(strncmp(frase,comandos[5],tam) == 0)
        {
            printf("Comando encerra\n");

            return 6;
        }

    }

    return 0;
}

void acorda(int s, siginfo_t *info, void* uc){

}


void* mostraListas(void* dados){

    balcao *td = (balcao* ) dados;
    do {
        sleep(td->tempo);
        printf("\nClientes: ");
        for (int i = 0; i < td->ite_cli; ++i) {
            printf("\nCliente %d:", i);
            printf("\n\tPID: %d", td->p_cli[i].pid_cli);
            printf("\n\tClassificacao: %s %d", td->p_cli[i].classificacao, td->p_cli[i].prio);
            printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_cli[i].com);
        }
        for (int i = 0; i < td->ite_med; ++i) {
            printf("\nMedico %d:", i);
            printf("\n\tPID: %d", td->p_med[i].pid_med);
            printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_med[i].com);
            ++td->p_med[i].temp;
        }
    }while(td->continua == 1);

    pthread_exit(NULL);
}


int main(int argc, char* argv[], char* envp[]) {
    int i = 0, canal[2], retorno[2], res, res_fork, res_pipe, estado, var, n_write, maxfd;
    int n_fifo, fd_canal, fd_cli, res_com, fd_med, fd_sinal;
    char str[100], str1[40], str_cli[40], str_com[40], str_med[40];


    struct timeval tempo;
    fd_set fds;
    pthread_t tid;  //Thread
    balcao b;   //Estrutura balcao
    b.ite_cli = 0; b.ite_med = 0;
    pedido p;   //Estrutura Pedido

    struct sigaction act;

    act.sa_sigaction = acorda;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);


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


    if (access(FIFO_SERV, F_OK) == 0) {
        printf("[ERRO] O FIFO já existe\n");
        exit(1);
    }
    mkfifo(FIFO_SERV, 0600);

    if (access(FIFO_SINAL, F_OK) == 0) {
        printf("[ERRO] O FIFO já existe\n");
        exit(1);
    }
    mkfifo(FIFO_SINAL, 0600);

    fd_canal = open(FIFO_SERV, O_RDWR);
    if (fd_canal == -1) {
        perror("\nImpossivel abrir o FIFO");
        exit(1);
    }

    fd_sinal = open(FIFO_SINAL, O_RDWR);
    if (fd_sinal == -1) {
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


    b.continua = 0; //Permitir correr os medico
    b.tempo = 20;
    pthread_create(&tid, NULL, mostraListas, (void* ) &b);

    do {
        printf("\nComandos: ");

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_canal, &fds);
        FD_SET(fd_sinal, &fds);

        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        maxfd = (fd_canal > fd_sinal) ? fd_canal : fd_sinal;

        res_com = select(maxfd+1, &fds, NULL, NULL, &tempo);
        if (res_com == 0) {
            printf("\nSem input...\n");
        } else if (res_com > 0) {
            if (FD_ISSET(0, &fds)) {
                fgets(str_com, sizeof(str_com), stdin);
                if (comandos(str_com) == 2) {
                    printf("\nA terminar o programa...\n");
                    break;
                }
            }
            if (FD_ISSET(fd_canal, &fds)) {

                n_fifo = read(fd_canal, &p, sizeof(pedido));
                if (n_fifo == -1) {
                    printf("\nNao conseguiu ler...\n");
                    exit(1);
                }
                if (n_fifo == sizeof(pedido)) {
                    b.continua = 1;         //Permitir correr a lista
                    if (p.cli_med == 0) {
                        if (b.ite_cli <= b.maxclientes - 1) {
                            n_write = write(canal[1], p.sintomas, strlen(p.sintomas));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...\n");
                                exit(1);
                            }
                            res = read(retorno[0], p.classificacao, sizeof(p.classificacao) - 1);
                            if (res == -1) {
                                printf("\nNão conseguiu ler...\n");
                                exit(1);
                            }
                            p.classificacao[res] = '\0';

                            //if(p.)
                            sprintf(str_cli, FIFO_CLI, p.pid_cli);
                            fd_cli = open(str_cli, O_WRONLY);
                            n_write = write(fd_cli, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...\n");
                                exit(1);
                            }
                            close(fd_cli);
                            b.p_cli[b.ite_cli] = p;
                            ++b.ite_cli;
                            printf("\nN. clientes: %d", b.ite_cli);
                            printf("\nSintomas: %s", b.p_cli[b.ite_cli-1].sintomas);
                        } else {
                            strcpy(p.classificacao, "O balcao nao consegue atender mais clientes...\n");
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
                        if (b.ite_med <= b.maxmedicos - 1) {
                            strcpy(p.msg, "esta ligado ao balcao!");
                            sprintf(str_med, FIFO_MED, p.pid_med);
                            fd_med = open(str_med, O_WRONLY);
                            n_write = write(fd_med, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_med);
                            b.p_med[b.ite_med] = p;
                            ++b.ite_med;
                            printf("\nN. medicos: %d", b.ite_med);
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
            if(FD_ISSET(fd_sinal, &fds)){
               int n = read(fd_sinal, &p , sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler...");
                    exit(1);
                }
               
            }
        }

        if(i == 0){
            if(b.ite_cli != 0 && b.ite_med != 0){
                b.p_cli[0].pid_med = b.p_med[0].pid_med;
                b.p_cli[0].com = 1;
                b.p_med[0].com = 1;
                b.p_cli[0].cli_med = 1;
                printf("\ncli: %d\tmed: %d\tmed_passado: %d", b.p_cli[0].pid_cli, b.p_med[0].pid_med, b.p_cli[0].pid_med);

                fd_cli = open(str_cli, O_WRONLY);
                n_write = write(fd_cli, &b.p_cli[0], sizeof(pedido));
                if(n_write == -1){
                    printf("\nNao conseguiu mandar a informação para o cliente...");
                    close(fd_cli);
                    break;
                }
                close(fd_cli);
                ++i;
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);


    close(fd_canal);
    unlink(FIFO_SERV);

    close(fd_sinal);
    unlink(FIFO_SINAL);

    p.sair = 1;
    b.continua = 0;
    pthread_kill(tid, SIGUSR2);
    pthread_join(tid, NULL);

    return 0;
}