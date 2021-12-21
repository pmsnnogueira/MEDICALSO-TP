//
// Created by rafael on 28/10/21.
//
#include <stdio.h>
#include "medico.h"

int main(int argc, char* argv[])
{
    medico a;
    if(argc < 3){
        printf("\nIndique o seu nome e a sua especialidade por parametro");
        return 1;
    }

    a.nome = argv[1];
    a.especialidade = argv[2];

    printf("\nOla %s da especialidade %s\n", a.nome, a.especialidade);

}