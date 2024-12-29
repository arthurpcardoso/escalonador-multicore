#include <stdio.h>

int main() {

    // Entrada do n_cores e quantum
    // formato: escalona <numero_de_cores> <quantum>

    int n_cores, quantum;
    char comando[10];

     // Ler o comando completo
    scanf("%s %d %d", comando, &n_cores, &quantum);

    // le o arquivo entrada.txt
    FILE *entrada = fopen("entrada.txt", "r");

    // le as linhas do arquivo
    char linha[100];
    while (fgets(linha, 100, entrada) != NULL) {
        char identificador[3], arquivo_executavel[50];
        int tempo_inicio, prioridade;

        sscanf(linha, "%s %s %d %d", identificador, arquivo_executavel, &tempo_inicio, &prioridade);

        int tempo = 0;

    // Procura o número ao final da string
        sscanf(arquivo_executavel, "%*[^0-9]%d", &tempo);

        printf("tempo: %d\n", tempo);
    }
    

    return 0;
}