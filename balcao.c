#include "balcao.h"

void mataThreads(int s, siginfo_t *info, void *uc){}

void* mostraListas(void* dados){

    balcao *td = (balcao* ) dados;
    do {
        sleep(td->tempo);
        pthread_mutex_lock(td->trinco);
        if(td->ite_cli > 0 || td->ite_med > 0){
            printf("\n\n************************** Listar Clientes e Medicos **************************\n");
            for (int i = 0; i < td->ite_cli; ++i) {
                printf("\nCliente %d:", i);
                printf("\n\tPID: %d", td->p_cli[i].pid_cli);
                printf("\n\tClassificacao: %s %d", td->p_cli[i].classificacao, td->p_cli[i].prio);
                printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_cli[i].com);
            }
            for (int i = 0; i < td->ite_med; ++i) {
                printf("\nMedico %d:", i);
                printf("\n\tPID: %d", td->p_med[i].pid_med);
                printf("\n\tEspecialidade: %s", td->p_med[i].especialidade);
                printf("\n\tTemporizador: %d", td->p_med[i].temp);
                printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", td->p_med[i].com);
                ++td->p_med[i].temp;
            }
            printf("\n\n*******************************************************************************\n");
        }
        pthread_mutex_unlock(td->trinco);
    }while(td->continua);

    pthread_exit(NULL);
}

void* apagaMed(void* dados){
    balcao *td = (balcao* ) dados;

    do {
        sleep(2);
        pthread_mutex_lock(td->trinco);

        if(td->ite_med > 0){
            for (int i = 0; i < td->ite_med; ++i) {
                if(td->p_med[i].temp <= 0){
                    for (int j = i; j < td->ite_med - 1; ++j) {
                        td->p_med[j] = td->p_med[j+1];
                    }
                    --td->ite_med;
                }else{
                    td->p_med[i].temp -= 2;
                }
            }
        }

        pthread_mutex_unlock(td->trinco);
    }while(td->continua);
    pthread_exit(NULL);
}


int main(int argc, char* argv[], char* envp[]) {
    int i = 0, canal[2], retorno[2], res, res_fork, res_pipe, estado, var, n_write, maxfd;
    int n_fifo, fd_canal, fd_cli, res_com, fd_med, fd_sinal;
    char str[100], str1[40], str_cli[40], str_com[40], str_med[40];


    struct timeval tempo;
    fd_set fds;
    pthread_t tid[2];  //Thread
    balcao b;   //Estrutura balcao
    b.ite_cli = 0; b.ite_med = 0;//"iteradores" dos arrays
    pedido p;   //Estrutura Pedido - enviada/recebida através dos FIFOS

    struct sigaction act;
    act.sa_sigaction = mataThreads;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);


    //Thread
    pthread_mutex_t trinco;
    if(pthread_mutex_init(&trinco, NULL) != 0) {
        printf("\nErro na inicialização do mutex\n");
        return 1;
    }


    setbuf(stdout, NULL);

    if (getenv("MAXCLIENTES") == NULL) {
        printf("Erro nas variaveis de Ambiente 'MAXCLIENTES'\n");
        return 0;
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
        return 0;
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

    //THREAD 1
    b.continua = 1; //Permitir Mostrar a lista de Clientes e Medicos
    b.tempo = 10;
    b.trinco = &trinco;
    pthread_create(&tid[0], NULL, mostraListas, (void* ) &b);

    //THREAD 3
    b.continua = 1; //Permitir apagar o medico
    b.tempo = 20;
    b.trinco = &trinco;
    pthread_create(&tid[1], NULL, apagaMed, (void* ) &b);

    do {
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


                char *ptr, str1[20];
                ptr = strtok(str_com, " ");
                strcpy(str1, ptr);
                ptr = strtok(NULL, " ");

                if (strcmp(str_com, "encerra\n") == 0) {
                    printf("\nA terminar o programa...\n");
                    strcpy(str_com, "sair\n");
                    break;
                }
                if(strcmp(str1, "utentes\n") == 0){
                    pthread_mutex_lock(&trinco);
                    if(b.ite_cli == 0){
                        printf("\nNao ha clientes");
                        continue;
                    }
                    for (int j = 0; j < b.ite_cli; ++j) {
                        printf("\nCliente %d:", i);
                        printf("\n\tPID: %d",b.p_cli[i].pid_cli);
                        printf("\n\tClassificacao: %s %d", b.p_cli[i].classificacao, b.p_cli[i].prio);
                        printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", b.p_cli[i].com);
                    }
                    pthread_mutex_unlock(&trinco);
                }
                if(strcmp(str1, "especialistas\n") == 0){
                    pthread_mutex_lock(&trinco);
                    if(b.ite_med == 0){
                        printf("\nNao ha medicos");
                        continue;
                    }
                    for (int j = 0; j < b.ite_med; ++j) {
                        printf("\nMedico %d:", i);
                        printf("\n\tPID: %d", b.p_med[i].pid_med);
                        printf("\n\tEspecialidade: %s", b.p_med[i].especialidade);
                        printf("\n\tTemporizador: %d", b.p_med[i].temp);
                        printf("\n\tEsta em consulta? (0->nao | 1->sim): %d", b.p_med[i].com);
                    }
                    pthread_mutex_unlock(&trinco);
                }

                if(strcmp(str1, "delut") == 0){

                    pthread_mutex_lock(&trinco);

                    int x = atoi(ptr);
                    int flag = 0;

                    for (int j = 0; j < b.ite_cli; ++j) {
                        if(b.p_cli[j].pid_cli == x && b.p_cli[i].com == 0){
                            p = b.p_cli[j];
                            flag = 1;
                            for (int k = j; k < b.ite_cli-1; ++k) {
                                b.p_cli[k] = b.p_cli[k+1];
                            }
                            --b.ite_cli;
                        }else{
                            printf("\nO cliente ou está em consulta ou então não existe");
                        }
                    }

                    pthread_mutex_unlock(&trinco);

                    if(flag == 1){
                        p.cli_med = 1;
                        strcpy(p.msg, "acabou\n");
                        sprintf(str_cli, FIFO_CLI, p.pid_cli);
                        fd_cli = open(str_cli, O_WRONLY);
                        if(fd_cli == -1){
                            printf("\nNao conseguiu abrir o  pipe do cliente...");
                            exit(1);
                        }
                        n_write = write(fd_cli, &p, sizeof(pedido));
                        if(n_write == -1){
                            printf("\nNao conseguiu escrever para o cliente...");
                            exit(1);
                        }
                        close(fd_cli);
                    }
                }

                if(strcmp(str_com, "delesp") == 0){
                    pthread_mutex_lock(&trinco);

                    int x = atoi(ptr);
                    int flag = 0;

                    for (int j = 0; j < b.ite_med; ++j) {
                        if(b.p_med[j].pid_med == x && b.p_med[i].com == 0){
                            p = b.p_med[j];
                            flag = 1;
                            for (int k = j; k < b.ite_med-1; ++k) {
                                b.p_med[k] = b.p_med[k+1];
                            }
                            --b.ite_med;
                        }else{
                            printf("\nO medico ou está em consulta ou então não existe");
                        }
                    }

                    pthread_mutex_unlock(&trinco);

                    if(flag == 1){
                        p.sair = 1;
                        sprintf(str_med, FIFO_MED, p.pid_med);
                        fd_med = open(str_med, O_WRONLY);
                        if(fd_cli == -1){
                            printf("\nNao conseguiu abrir o  pipe do cliente...");
                            exit(1);
                        }
                        n_write = write(fd_med, &p, sizeof(pedido));
                        if(n_write == -1){
                            printf("\nNao conseguiu escrever para o cliente...");
                            exit(1);
                        }
                        close(fd_cli);
                    }
                }
                if(strcmp(str_com, "freq") == 0){

                    int x = atoi(ptr);
                    pthread_mutex_lock(&trinco);

                    b.tempo = x;
                    printf("\nMudou a frequencia para %d segundos", b.tempo);

                    pthread_mutex_unlock(&trinco);

                }



                }
            }



            if (FD_ISSET(fd_canal, &fds)) {

                n_fifo = read(fd_canal, &p, sizeof(pedido));
                if (n_fifo == -1) {
                    printf("\nNao conseguiu ler...\n");
                    exit(1);
                }

                if (n_fifo == sizeof(pedido)) {
                    if (p.cli_med == 0) {
                        int cli_existe = 0;

                        pthread_mutex_lock(&trinco);

                        for (int j = 0; j < b.ite_cli; ++j) {
                            if(b.p_cli[j].pid_cli == p.pid_cli){
                                b.p_cli[j].com = 0;
                                cli_existe = 1;
                                break;
                            }
                        }

                        int i_cli = b.ite_cli;
                        int max_cli = b.maxclientes;
                        pthread_mutex_unlock(&trinco);


                        if(cli_existe == 1){
                            cli_existe = 0;
                            continue;
                        }

                        if (i_cli <= max_cli - 1) {
                            //escreve para o classificador
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
                            p.cli_med = 0;//porque é cliente

                            sprintf(str_cli, FIFO_CLI, p.pid_cli);
                            fd_cli = open(str_cli, O_WRONLY);
                            n_write = write(fd_cli, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...\n");
                                exit(1);
                            }
                            close(fd_cli);

                            char * ptr;
                            char aux_str[50];
                            ptr = strtok(p.classificacao, " ");
                            strcpy(aux_str, ptr);
                            ptr = strtok(NULL, " ");
                            strcpy(p.classificacao, aux_str);
                            p.prio = atoi(ptr);

                            //------------CICLO LIGAÇÃO AO MÉDICO (É UM CLIENTE)---------------

                            int lig_c = 0;

                            pthread_mutex_lock(&trinco);

                            for (int j = 0; j < b.ite_med; ++j) {
                                if(strcmp(b.p_med[j].especialidade, p.classificacao) == 0 && b.p_med[j].com == 0){
                                    lig_c = 1;
                                    b.p_med[j].com = 1;
                                    p.pid_med = b.p_med[j].pid_med;
                                    p.com = 1;
                                    printf("\nEncontrou um medico (%d) para o atender", b.p_med[j].pid_med);
                                }else{
                                    p.com = 0;
                                    printf("\nNao encontrou nenhum cliente");
                                }
                            }

                            p.cli_med = 0;
                            b.p_cli[b.ite_cli] = p;//copia para o array dos clientes
                            ++b.ite_cli;

                            pthread_mutex_unlock(&trinco);

                            if(lig_c == 1){
                                printf("\nVai ligar");
                                p.cli_med = 0;//para simular que foi o medico que ligou
                                sprintf(str_med, FIFO_MED, p.pid_med);
                                fd_med = open(str_med, O_WRONLY);
                                n_write = write(fd_med, &p, sizeof(pedido));
                                if (n_write == -1) {
                                    printf("\nNão conseguiu escrever...");
                                    exit(1);
                                }
                                close(fd_med);
                            }

                            //------------FIM CICLO LIGAÇÃO AO MÉDICO (É UM CLIENTE)---------------

                        } else {
                            strcpy(p.classificacao, "O balcao nao consegue atender mais clientes...\n");
                            p.cli_med = 0;
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
                        int med_existe = 0;

                        pthread_mutex_lock(&trinco);

                        for (int j = 0; j < b.ite_cli; ++j) {
                            if(b.p_med[j].pid_med == p.pid_med){
                                b.p_med[j].com = 0;
                                med_existe = 1;
                                break;
                            }
                        }

                        int i_med = b.ite_med;
                        int max_med = b.maxmedicos;

                        pthread_mutex_unlock(&trinco);

                        if(med_existe == 1){
                            med_existe = 0;
                            continue;
                        }

                        if (i_med<= max_med - 1) {
                            strcpy(p.msg, "esta ligado ao balcao!");
                            p.cli_med = 1;
                            sprintf(str_med, FIFO_MED, p.pid_med);
                            fd_med = open(str_med, O_WRONLY);
                            n_write = write(fd_med, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_med);


                            //------------CICLO LIGAÇÃO AO CLIENTE (É UM MÉDICO)--------------

                            int lig_m = 0;

                            pthread_mutex_lock(&trinco);

                            for (int j = 0; j < b.ite_cli; ++j) {
                                if(strcmp(b.p_cli[j].classificacao, p.especialidade) == 0 && b.p_cli[j].com == 0){
                                    lig_m = 1;
                                    b.p_cli[j].com = 1;
                                    p.pid_cli = b.p_cli[j].pid_cli;
                                    p.com = 1;
                                    printf("\nEncontrou um cliente (%d) que precisa de ser atendido", b.p_cli[j].pid_cli);
                                }else{
                                    p.com = 0;
                                    printf("\nNao encontrou nenhum cliente");
                                }
                            }

                            p.cli_med = 1;
                            b.p_med[b.ite_med] = p;//copia para o array dos clientes
                            ++b.ite_med;

                            pthread_mutex_unlock(&trinco);

                            if(lig_m == 1){
                                printf("\nVai ligar");
                                p.cli_med = 1;//para simular que foi o medico que ligou
                                sprintf(str_cli, FIFO_CLI, p.pid_cli);
                                fd_cli = open(str_cli, O_WRONLY);
                                n_write = write(fd_cli, &p, sizeof(pedido));
                                if (n_write == -1) {
                                    printf("\nNão conseguiu escrever (ligacao para o cliente)...");
                                    exit(1);
                                }
                                close(fd_cli);
                            }

                            //------------FIM CICLO LIGAÇÃO AO CLIENTE (É UM MÉDICO)--------------

                        } else {
                            strcpy(p.msg, "O balcao nao tem capacidade para mais medicos...");
                            sprintf(str_med, FIFO_MED, p.pid_med);
                            fd_med = open(str_med, O_WRONLY);
                            p.cli_med = 1;
                            p.com = 5;
                            n_write = write(fd_med, &p, sizeof(pedido));
                            if (n_write == -1) {
                                printf("\nNão conseguiu escrever...");
                                exit(1);
                            }
                            close(fd_med);
                            continue;
                        }
                        pthread_mutex_unlock(&trinco);
                    }
                }
            }
            if(FD_ISSET(fd_sinal, &fds)){
                int n = read(fd_sinal, &p, sizeof(pedido));
                if (n == -1) {
                    printf("\nNao conseguiu ler...");
                    exit(1);
                }

                pthread_mutex_lock(&trinco);

                if(p.cli_med == 1){//medico quer sair
                    for (int j = 0; j < b.ite_med; ++j) {
                        if(b.p_med[i].pid_med == p.pid_med){
                            b.p_med[i].temp = 20;//deu sinal de vida, então repoe o temporizador
                        }
                    }
                }else{
                    if(p.sair == 1){//cliente quer sair
                        for (int j = 0; j < b.ite_cli; ++j) {
                            if(b.p_cli[j].pid_cli == p.pid_cli){
                                for (int k = j; k < b.ite_cli - 1; ++k) {
                                    b.p_cli[k] = b.p_cli[k+1];
                                }
                                --b.ite_cli;
                            }
                        }
                    }
                }

                pthread_mutex_unlock(&trinco);
            }

}while(strcmp(str_com, "encerra\n") != 0);

    p.sair = 1;
    b.continua = 0;

    close(fd_canal);
    unlink(FIFO_SERV);

    close(fd_sinal);
    unlink(FIFO_SINAL);


    pthread_kill(tid[0], SIGUSR2);
    pthread_join(tid[0], NULL);

    pthread_kill(tid[1], SIGUSR2);
    pthread_join(tid[1], NULL);

    pthread_mutex_destroy(&trinco);

    return 0;
}