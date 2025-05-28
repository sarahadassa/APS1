// eventos.h
#ifndef EVENTOS_H
#define EVENTOS_H

#include "comum.h" // Inclui as definições e constantes globais

// Protótipos das funções de gerenciamento de eventos
void cadastrarEvento(); // Esta função virá do menuAdmin, mas encapsulará a lógica
void listarEventos();
void bubbleSort();
Evento* merge(Evento *a, Evento *b); // Helper para mergeSort
void split(Evento *source, Evento **front, Evento **back); // Helper para mergeSort
void mergeSort(Evento **head);
void atualizarIndice();
Evento* buscaIndexada(char *nome);

#endif // EVENTOS_H
