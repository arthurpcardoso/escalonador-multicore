#include "processos.h" // Arquivo com a definição da estrutura `Processo`
#include <unistd.h>    // Para a função `sleep`
#include <stdio.h>     // Para entrada e saída padrão

#define MAX_PROCESSOS 100 // Máximo de processos por fila
#define NUM_PRIORIDADES 4 // Número de níveis de prioridade (0 a 3)

Processo processos_concluidos[MAX_PROCESSOS];
int index_concluidos = 0;

// Estrutura para armazenar processos por prioridade
typedef struct {
    Processo processos[MAX_PROCESSOS]; // Vetor de processos
    int count;                         // Contador de processos na fila
} FilaPrioridade;

// Vetor de filas de prioridade
FilaPrioridade filas[NUM_PRIORIDADES];

// Inicializa todas as filas de prioridade
void inicializar_filas() {
    for (int i = 0; i < NUM_PRIORIDADES; i++) {
        filas[i].count = 0; // Define a contagem inicial como 0
    }
}

// Adiciona processos às filas correspondentes baseando-se em suas prioridades
void adicionar_processos(Processo processos[], int num_processos) {
    for (int i = 0; i < num_processos; i++) {
        int prioridade = processos[i].prioridade;
        // Verifica se a prioridade está dentro do intervalo permitido
        if (prioridade >= 0 && prioridade < NUM_PRIORIDADES) {
            // Adiciona o processo à fila correspondente
            filas[prioridade].processos[filas[prioridade].count++] = processos[i];
        }
    }
}

void executar_processos() {
    int tempo_total = 0;
    int quantum = 1;

    while (1) {
        int fila_vazia = 1;

        for (int prioridade = 0; prioridade < NUM_PRIORIDADES; prioridade++) {

            FilaPrioridade *fila = &filas[prioridade];

            for (int i = 0; i < fila->count; i++) {
                Processo *p = &fila->processos[i];

                printf("Executando processo ID: %d, Prioridade: %d\n", p->id, prioridade);
                sleep(quantum);

                // Calcula tempos de término e espera
                p->tempo_termino = tempo_total + quantum;
                p->tempo_espera = p->tempo_termino - p->tempo_inicio - quantum;

                // Adiciona o processo à lista de concluídos
                processos_concluidos[index_concluidos++] = *p;

                printf("Processo ID: %d concluído. Tempo de Turnaround: %d, Tempo de Espera: %d\n",
                       p->id, p->tempo_termino - p->tempo_inicio, p->tempo_espera);

                tempo_total += quantum;
                fila_vazia = 0;
                break; // Sai do loop da fila atual
            }
        }

        if (fila_vazia) {
            break; // Sai do loop principal se todas as filas estiverem vazias
        }
    }

    // Imprime a lista de processos concluídos
    printf("\nProcessos concluídos na ordem de finalização:\n");
    for (int j = 0; j < index_concluidos; j++) {
        printf("ID: %d, Tempo de Turnaround: %d, Tempo de Espera: %d\n",
               processos_concluidos[j].id,
               processos_concluidos[j].tempo_termino - processos_concluidos[j].tempo_inicio,
               processos_concluidos[j].tempo_espera);
    }
}
