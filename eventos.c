#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 50
#define MAX_PART 3
#define MAX_EVENTOS 100
#define HASH_SIZE 100

// cores escolhidas para sair no terminal
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

typedef struct Participante { int id; char nome[MAX]; struct Participante *prox; } Part;
typedef struct Inscricao { int evento_id, participante_id; char nome_participante[MAX]; struct Inscricao *prox; } Inscricao;// relacionando ID e nome
typedef struct Evento { int id, vagas; char nome[MAX], categoria[MAX]; Part *participantes; struct Evento *prox; } Evento;
typedef struct FilaEspera { Part *participante; int evento_id; struct FilaEspera *prox; } FilaEspera;


Evento *eventos = NULL; Inscricao *todas_inscricoes = NULL; FilaEspera *inicio_fila = NULL, *fim_fila = NULL;
Part *hash_table[HASH_SIZE] = {NULL}; int next_evento_id = 1, next_part_id = 1, total_eventos = 0;


void bubbleSort(), mergeSort(Evento**), insereParticipante(Evento*, Part*), insereFilaEspera(int, Part*), liberarMemoria();
Evento* merge(Evento*, Evento*); int hashParticipante(char*), buscaParticipante(Part*, char*);
void split(Evento*, Evento**, Evento**), listarEventos(), listarTodasInscricoes(), listarFilaEspera();


void bubbleSort() {
    if (!eventos || !eventos->prox) return;
    int trocado; Evento **ptr, *temp, *ultimo = NULL;
    do {
        trocado = 0; ptr = &eventos;
        while ((*ptr)->prox != ultimo) {
            int cmp_cat = strcmp((*ptr)->categoria, (*ptr)->prox->categoria);
            int cmp_nome = strcmp((*ptr)->nome, (*ptr)->prox->nome);
            if (cmp_cat > 0 || (cmp_cat == 0 && cmp_nome > 0)) {
                temp = *ptr; *ptr = (*ptr)->prox; temp->prox = (*ptr)->prox; (*ptr)->prox = temp; trocado = 1;// troca os nós de posiçao
            } ptr = &(*ptr)->prox;
        } ultimo = *ptr;
    } while (trocado);
}
