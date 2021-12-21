//
// Created by rafael on 28/10/21.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "balcao.h"
#include <sys/wait.h>

int comandos(char *frase){
	int tam = strlen(frase) -1;
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
             break;
         }

     }

	return 0;
}


int main(int argc, char* argv[], char* envp[]){
    int i, canal[2], retorno[2], res, res_fork, res_pipe, estado;
    int var;
    char str[100], str1[40];
    balcao b;

    setbuf(stdout, NULL);

    if(getenv("MAXCLIENTES") == NULL)
    {
        printf("Erro nas variaveis de Ambiente 'MAXCLIENTES'\n");

    }
    else
    {
        var = atoi(getenv("MAXCLIENTES"));
        if(var > 0)
        {
            b.maxclientes = var;
            printf("O valor de maxclientes e: %d", b.maxclientes);
        }else
        {
            printf("O valor tem de ser superior a 0 \n");
        }

	}

    if(getenv("MAXMEDICOS") == NULL)
    {
        printf("Erro nas variaveis de ambiente 'MAXMEDICOS'\n");

    }
    else
    {
        var = atoi(getenv("MAXMEDICOS"));
        if(var > 0){
            b.maxmedicos = var;
            printf("\nO valor de maxmedicos e: %d\n", b.maxmedicos);
        }else{
            printf("O valor tem de ser superior a 0 \n");
        }
    }


    res_pipe = pipe(canal);
    if(res_pipe < 0){
        perror("\nerro pipe1: ");
        return 1;
    }

    res_pipe=pipe(retorno);
    if(res_pipe < 0){
        perror("\nerro pipe2: ");
        return 1;
    }

    res_fork = fork();
    if(res_fork < 0){
        perror("\nerro fork: ");
        return 1;
    }
    if(res_fork == 0){//FILHO
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
    while (1){
        printf("\nSintomas: ");
        fgets(str1, sizeof(str1) - 1, stdin);
        write(canal[1], str1, strlen(str1));
		if(comandos(str1) == 2)
            break;
        res = read(retorno[0], str, sizeof(str) - 1);
        str[res] = '\0';
        strcpy(b.a.sintomas, str);
        printf("\nO diagnostico do cliente e: %s\n", b.a.sintomas);
    }

    wait(&estado);
    close(canal[1]);
    close(retorno[0]);

    return 0;
}
