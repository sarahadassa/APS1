// participantes.h
#ifndef PARTICIPANTES_H
#define PARTICIPANTES_H

#include "comum.h" // Inclui as definições e constantes globais

// Protótipos das funções de gerenciamento de participantes
int hashParticipante(char *email);
Part* buscaParticipantePorEmailNaHash(char *email); // Nova função para buscar na hash

#endif // PARTICIPANTES_H
