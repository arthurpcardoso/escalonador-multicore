#include "processos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSOS 100

int ler_processos(const char *nome_arquivo, Processo processos[]) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir o arquivo: %s\n", nome_arquivo);
        return -1;
    }

    int count = 0;
    while (count < MAX_PROCESSOS && fscanf(arquivo, "%d %s %d %d", 
            &processos[count].id, 
            processos[count].arquivo, 
            &processos[count].tempo_inicio, 
            &processos[count].prioridade) == 4) {
        count++;
    }

    fclose(arquivo);
    return count;
}