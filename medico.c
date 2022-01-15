//
// Created by rafael on 28/10/21.
//
#include "biblio.h"
#include "medico.h"



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


int main(int argc, char* argv[]) {
    int fd, n, fd_med = 0, n_med, res_com, fd_lig_m = 0, fd_lig_c = 0, maxfd, estado = 0;
    char str[40], str_lig_m[40], str_com[40], str_lig_c[40];
    struct timeval tempo;
    fd_set fds;
    medico m;
    pedido p;
    pthread_t tid;


    if (argc < 3) {
        printf("\nIndique o seu nome e a sua especialidade por parametro");
        return 1;
    }

    setbuf(stdout, NULL);

    pthread_mutex_t trinco;
    if (pthread_mutex_init(&trinco, NULL) != 0) {
        printf("\nErro na inicialização do mutex\n");
        return 1;
    }

    m.m.pid_med = getpid();
    printf("\n[PID:%d]Ola %s da especialidade %s\n", m.m.pid_med, argv[1], argv[2]);


    if (access("sinal", F_OK) != 0) {
        printf("\nO servidor esta desligado...\n");
        exit(1);
    }

    sprintf(str, FIFO_MED, m.m.pid_med);
    mkfifo(str, 0600);


    m.m.cli_med = 1;
    m.m.pid_cli = 0;
    m.m.com = 0;
    m.m.sair = 0;
    m.m.temp = 20;
    strcpy(m.m.especialidade, argv[2]);


    fd = open(FIFO_SERV, O_WRONLY);
    if(fd == -1){
        printf("\nNao abriu o FIFO do balcao");
        exit(1);
    }
    n = write(fd, &m.m, sizeof(pedido));
    if(n == -1){
        printf("\nNao conseguiu escrever");
        exit(1);
    }
    close(fd);

    fd = open(str, O_RDONLY);
    if(fd == -1){
        printf("\nNao abriu o FIFO do balcao");
        exit(1);
    }
    n = read(fd, &m.m, sizeof(pedido));
    if(n == -1){
        printf("\nNao conseguiu ler");
        exit(1);
    }
    close(fd);

    m.trinco = &trinco;
    pthread_create(&tid, NULL, temporizador, (void* ) &m.m);


    fd_med = open(str, O_RDWR | O_NONBLOCK);
    if(fd_med == -1){
        perror("\nImpossivel abrir o FIFO do medico");
        exit(1);
    }

    do {
        if(estado == 2){
            printf("R: ");
        }
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(fd_med, &fds);
        tempo.tv_sec = 10;
        tempo.tv_usec = 0;

        res_com = select(fd_med+1, &fds, NULL, NULL, &tempo);

        if(res_com == 0){
            printf("\nA espera de clientes...");
        }else if(res_com > 0){
            if(FD_ISSET(0, &fds)){
                fgets(str_com, sizeof(str_com), stdin);

                pthread_mutex_lock(&trinco);
                strcpy(m.m.msg, str_com);
                pthread_mutex_unlock(&trinco);

                if(estado == 1){
                    fd = open(FIFO_SERV, O_WRONLY);
                    if(fd == -1){
                        printf("\nNao conseguiu abrir o FIFo do balcao...");
                        exit(1);
                    }


                    pthread_mutex_lock(&trinco);
                    m.m.cli_med = 1;
                    p = m.m;
                    pthread_mutex_lock(&trinco),
                            write(fd, &p, sizeof(pedido));
                    close(fd);
                }else{
                    pthread_mutex_lock(&trinco);
                    sprintf(str_lig_c, FIFO_CLI, m.m.pid_cli);
                    m.m.cli_med = 1;
                    pthread_mutex_unlock(&trinco);

                    if(access(str_lig_c, F_OK) == 0){
                        pthread_mutex_lock(&trinco);
                        p = m.m;
                        pthread_mutex_unlock(&trinco);
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

                    pthread_mutex_lock(&trinco);
                    m.m = p;
                    pthread_mutex_unlock(&trinco);

                }else{
                    estado = 1;

                    pthread_mutex_lock(&trinco);

                    m.m = p;
                    if (strcmp(m.m.msg, "o balcao fechou") == 0) {
                        printf("%s", m.m.msg);
                        strcpy(str_com, "sair\n");
                    }
                    pthread_mutex_unlock(&trinco);
                }
            }
        }
    }while(strcmp(str_com, "sair\n") != 0);



    close(fd_med);
    close(fd_lig_m);

    pthread_mutex_lock(&trinco);
    m.m.sair = 1;
    pthread_mutex_unlock(&trinco);
    pthread_join(tid, NULL);

    unlink(str);
    unlink(str_lig_m);

    pthread_mutex_destroy(&trinco);
    exit(0);
}
