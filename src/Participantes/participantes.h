// participantes.h
#ifndef PARTICIPANTES_H
#define PARTICIPANTES_H

#include "comum.h"

// Protótipos das funções de gerenciamento de participantes
int hashParticipante(char *email);
Part* buscaParticipantePorEmailNaHash(char *email); // Nova função para buscar na hash
Part* obterOuCriarParticipante(char *email_usuario); // <-- Adicionando este protótipo aqui

#endif // PARTICIPANTES_H
