// fila_pilha.h
#ifndef FILA_PILHA_H
#define FILA_PILHA_H

#include "comum.h" 

// Protótipos das funções da Fila de Espera
void insereFilaEspera(Evento *e, Part *p);
void listarFilaEspera();

// Protótipos das funções da Pilha de Eventos
void pushEvento(Evento *e);
Evento* popEvento();
void listarPilhaEventos();

#endif // FILA_PILHA_H
