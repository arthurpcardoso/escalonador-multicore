#include <stdio.h>
#include <stdlib.h>
#include "processos.h"

#define MAX_PROCESSOS 100

Processo processos[MAX_PROCESSOS];

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <numero_de_cores> <quantum>\n", argv[0]);
        return 1;
    }

    int numero_de_cores = atoi(argv[1]);
    int quantum = atoi(argv[2]);

    if (numero_de_cores <= 0 || quantum <= 0) {
        fprintf(stderr, "Erro: <numero_de_cores> e <quantum> devem ser maiores que zero.\n");
        return 1;
    }

    printf("Número de núcleos: %d, Quantum: %d\n", numero_de_cores, quantum);

    int num_processos = ler_processos("processos.txt", processos);

    if (num_processos < 0) {
        return 1; 
    }

    adicionar_processos(processos, num_processos);

    executar_processos();

    return 0;
}