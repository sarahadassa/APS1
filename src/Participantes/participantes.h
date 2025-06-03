#ifndef PARTICIPANTES_H
#define PARTICIPANTES_H

#include "comum.h"

// Inicializa a tabela hash de participantes
void inicializarHashParticipantes();

// Protótipos das funções
int hashParticipante(const char *email);
Part* buscaParticipantePorEmailNaHash(const char *email);
Part* obterOuCriarParticipante(const char *email_usuario);

#endif // PARTICIPANTES_H