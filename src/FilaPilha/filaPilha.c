// FilaPilha/filaPilha.c
#include "FilaPilha/filaPilha.h" // Inclui os protótipos do próprio módulo
#include "Evento/eventos.h"    // Necessário para acessar a variável global 'eventos' na função listarFilaEspera

// Implementação da função para inserir um participante na fila de espera
void insereFilaEspera(Evento *e, Part *p) {
    FilaEspera *novo = malloc(sizeof(FilaEspera));
    if (!novo) {
        printf(RED "Erro ao alocar memoria para fila de espera!\n" RESET);
        return;
    }
    novo->evento_id = e->id;
    novo->participante = p;
    novo->prox = NULL;
    if (!inicio_fila) {
        inicio_fila = fim_fila = novo;
    } else {
        fim_fila->prox = novo;
        fim_fila = novo;
    }
    printf(YELLOW "%s (%s) adicionado a fila de espera do evento %s\n" RESET, p->nome, p->email, e->nome);
}

// Implementação da função para listar a fila de espera
void listarFilaEspera() {
    if (!inicio_fila) {
        printf("\n" YELLOW "Fila de espera vazia!\n" RESET);
        return;
    }
    printf("\n" MAGENTA "=== FILA DE ESPERA ===\n" RESET);
    FilaEspera *atual = inicio_fila;
    while (atual) {
        // Encontra o evento pelo ID (precisa da lista global de eventos)
        Evento *e = eventos; // 'eventos' é uma variável global de comum.h
        while (e && e->id != atual->evento_id) e = e->prox;
        if (e) {
            printf(CYAN "Evento: %s | Participante: %s (%s)\n" RESET, e->nome, atual->participante->nome, atual->participante->email);
        } else {
            printf(CYAN "Evento (ID: %d - NAO ENCONTRADO) | Participante: %s (%s)\n" RESET, atual->evento_id, atual->participante->nome, atual->participante->email);
        }
        atual = atual->prox;
    }
}

// Implementação da função para adicionar um evento à pilha (push)
void pushEvento(Evento *e) {
    PilhaEventos *novo = malloc(sizeof(PilhaEventos));
    if (!novo) {
        printf(RED "Erro ao alocar memoria para pilha!\n" RESET);
        return;
    }
    novo->evento = e;
    novo->prox = topo_pilha;
    topo_pilha = novo;
}

// Implementação da função para remover um evento do topo da pilha (pop)
Evento* popEvento() {
    if (!topo_pilha) return NULL;
    PilhaEventos *temp = topo_pilha;
    Evento *e = temp->evento;
    topo_pilha = topo_pilha->prox;
    free(temp);
    return e;
}

// Implementação da função para listar os eventos na pilha
void listarPilhaEventos() {
    if (!topo_pilha) {
        printf(YELLOW "\nPilha de eventos vazia!\n" RESET);
        return;
    }
    printf(MAGENTA "\n=== PILHA DE EVENTOS (HISTORICO) ===\n" RESET);
    PilhaEventos *atual = topo_pilha;
    while (atual) {
        printf(CYAN "Evento: %s (ID: %d)\n" RESET, atual->evento->nome, atual->evento->id);
        atual = atual->prox;
    }
}
