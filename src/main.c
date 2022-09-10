#include <stdio.h>
#include <unistd.h>
int main(int argc, char* argv[]) {

    if(argc < 2){
        printf("\nIndique o nome do balcao por parametro.");
        return 1;
    }


    execl(argv[1],argv[1], NULL);

    return 0;
}
