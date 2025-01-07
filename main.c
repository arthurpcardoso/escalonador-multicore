#include "fila.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/*
    Alunos:
    - Arthur Pereira Cardoso 211038191
    - Arthur Rattes 200053523
    - José Barbosa 202014305
    - Victor Hugo Da Silva 190129794
*/

typedef struct {
  int identificador;
  char arquivo_executavel[50];
  int tempo_inicio;
  int prioridade;
  int tempo_execucao;
  int tempo_restante;
  int tempo_final;
  bool liberado_executar;
} Processo;

Fila fila_prioridade_0;
Fila fila_prioridade_1;
Fila fila_prioridade_2;
Fila fila_prioridade_3;

int prox_processo();

int verifica_processo_nasceu(Processo *processos, int tempo,
                             int qtd_processos); // Function prototype

void quantum_acabou(Processo *processos, int qtd_processos);

void terminou_processo(int pid);

void verifica_processo_terminou(Processo *processos, int tempo,
                                int qtd_processos);

bool verifica_todos_processos_terminaram(Processo *processos,
                                         int qtd_processos);

int main() {

  // Entrada do n_cores e quantum
  // formato: escalona <numero_de_cores> <quantum>

  int n_cores, quantum;
  char comando[10];

  // Ler o comando completo
  scanf("%s %d %d", comando, &n_cores, &quantum);

  FILE *entrada = fopen("entrada.txt", "r");

  // le o arquivo entrada.txt
  if (!entrada) {
    perror("Erro ao abrir o arquivo");
    return 1;
  }

  // Contar as linhas do arquivo
  int num_linhas = 0;
  char linha[100];
  while (fgets(linha, sizeof(linha), entrada)) {
    num_linhas++;
  }

  // Voltar ao início do arquivo
  rewind(entrada);

  printf("Número de linhas: %d\n", num_linhas);

  // Criar vetor de processos (ou outro armazenamento)
  Processo processos[num_linhas];

  // Iterar exatamente pela quantidade de linhas do arquivo
  for (int i = 0; i < num_linhas; i++) {
    if (fgets(linha, sizeof(linha), entrada)) {
      Processo p;
      p.tempo_final = 0;

      sscanf(linha, "%d %s %d %d", &p.identificador, p.arquivo_executavel,
             &p.tempo_inicio, &p.prioridade);

      sscanf(p.arquivo_executavel, "%*[^0-9]%d", &p.tempo_execucao);

      p.tempo_restante = p.tempo_execucao;

      // Coloca o processo no vetor
      processos[p.identificador] = p;
    }
  }

  int tempo = 0;
  int tempo_quantum = 0;

  // execução do escalonador
  while (1) {
    printf("-----------------Tempo: %d-----------------------\n", tempo);
    // verifica se algum processo chegou
    verifica_processo_nasceu(processos, tempo, num_linhas);

    // Verifica se tem core disponivel
    if (n_cores > 0) {
      while (n_cores > 0) {
        int processo = prox_processo();
        if (processo != 0) {
          printf("Executando processo %d\n", processo);
          int pid = fork();
          if (pid == 0) {
            while (processos[processo].tempo_restante > 0) {
              if (processos[processo].liberado_executar) {
                processos[processo].tempo_restante--;
                printf("Processo %d executando\n", processo);
              } else {
                printf("Processo %d bloqueado\n", processo);
                while (!processos[processo].liberado_executar)
                  ;
              }
            }
            printf("Processo %d terminou\n", processo);
            n_cores--;
          } else {
            break;
          }
        }
      }

      // verifica se algum processo terminou
      verifica_processo_terminou(processos, tempo, num_linhas);

      // verifica se o quantum acabou
      if (tempo_quantum == quantum) {
        quantum_acabou(processos, num_linhas);
        tempo_quantum = 0;
      }

      // verifica se todos os processos terminaram
      if (verifica_todos_processos_terminaram(processos, num_linhas)) {
        break;
      }

      // delay de 1 segundo
      long delay;
      for (delay = 0; delay < 600000000; delay++)
        ;

      tempo++;
      tempo_quantum++;
    }
  }

  return 0;
}

int verifica_processo_nasceu(Processo *processos, int tempo,
                             int qtd_processos) {
  // verifica na tabela de processos quais processos nasceram
  // e coloca na fila de prioridade correspondente

  for (int i = 0; i < qtd_processos; i++) {
    if (processos[i].identificador == 0) {
      continue;
    }
    if (processos[i].tempo_inicio == tempo) {
      if (processos[i].prioridade == 0) {
        enfileirar(&fila_prioridade_0, i);
      } else if (processos[i].prioridade == 1) {
        enfileirar(&fila_prioridade_1, i);
      } else if (processos[i].prioridade == 2) {
        enfileirar(&fila_prioridade_2, i);
      } else if (processos[i].prioridade == 3) {
        enfileirar(&fila_prioridade_3, i);
      }
    }
  }
  return 0;
}

void quantum_acabou(Processo *processos, int qtd_processos) {
  // coloca os processos em execução de volta na fila de prioridade

  // verfica quais processos estão em execução
  for (int i = 0; i < qtd_processos; i++) {
    if (processos[i].liberado_executar) {
      processos[i].liberado_executar = false;
      if (processos[i].prioridade == 0) {
        enfileirar(&fila_prioridade_0, i);
      } else if (processos[i].prioridade == 1) {
        enfileirar(&fila_prioridade_1, i);
      } else if (processos[i].prioridade == 2) {
        enfileirar(&fila_prioridade_2, i);
      } else if (processos[i].prioridade == 3) {
        enfileirar(&fila_prioridade_3, i);
      }
    }
  }
}

void verifica_processo_terminou(Processo *processos, int tempo,
                                int qtd_processos) {
  // verifica na tabela de processos quais processos terminaram
  // e coloca na fila de prioridade correspondente

  for (int i = 0; i < qtd_processos; i++) {
    if (processos[i].tempo_restante == 0) {
      processos[i].tempo_final = tempo;
    }
  }
}

bool verifica_todos_processos_terminaram(Processo *processos,
                                         int qtd_processos) {
  // verifica se todos os processos terminaram

  for (int i = 0; i < qtd_processos; i++) {
    if (processos[i].tempo_restante > 0) {
      return false;
    }
  }
  return true;
}

int prox_processo() {
  // retorna o proximo processo a ser executado

  if (!estaVazia(&fila_prioridade_0)) {
    printf("tirando da fila 0\n");
    return desenfileirar(&fila_prioridade_0);
  } else if (!estaVazia(&fila_prioridade_1)) {
    printf("tirando da fila 1\n");
    return desenfileirar(&fila_prioridade_1);
  } else if (!estaVazia(&fila_prioridade_2)) {
    printf("tirando da fila 2\n");
    return desenfileirar(&fila_prioridade_2);
  } else if (!estaVazia(&fila_prioridade_3)) {
    printf("tirando da fila 3\n");
    return desenfileirar(&fila_prioridade_3);
  } else {
    return -1;
  }
}