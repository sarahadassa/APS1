// persist.h
#ifndef PERSIST_H
#define PERSIST_H

#include "comum.h"       // Para as structs e variáveis globais
#include "utils.h"       // Para limparBuffer e validarEmail (usado indiretamente no carregamento)
#include "participantes.h" // Para hashParticipante (usado no carregamento de participantes)
#include "inscricoes.h"    // Para insereParticipante (usado no carregamento de inscrições)
#include "eventos.h"     // Para atualizarIndice (chamado após carregar tudo)


// Protótipos das funções de salvamento
void salvarEventosCSV();
void salvarParticipantesCSV();
void salvarInscricoesCSV();
void salvarFilaEsperaCSV();
void salvarTudoCSV();

// Protótipos das funções de carregamento
void carregarEventosCSV();
void carregarParticipantesCSV();
void carregarInscricoesCSV();
void carregarFilaEsperaCSV();
void carregarTudoCSV();

// Protótipo para liberar toda a memória
void liberarMemoria();

#endif // PERSIST_H
