#include "comum.h"
#include "Utils/utils.h"
#include "Evento/evento.h"
#include "participantes.h"
#include "Inscricoes/inscricoes.h"
#include "FilaPilha/filaPilha.h"
#include "Persistencia/persist.h"

// Implementação da função de hash para emails de participantes
int hashParticipante(char *email) {
    int sum = 0;
    for (int i = 0; email[i]; sum += email[i++]);
    return sum % HASH_SIZE;
}

// Implementação da busca de participante na hash table por email
// Esta função é uma adaptação e simplificação da sua lógica de busca na hash
// O código original tinha a lógica de busca dentro de insereParticipante.
// Esta função visa encapsular a busca na hash.
Part* buscaParticipantePorEmailNaHash(char *email) {
    if (!email || strlen(email) == 0) {
        return NULL;
    }
    int index = hashParticipante(email);
    // Para colisões simples onde você sobrescreve ou espera que seja o mesmo email
    if (hash_table[index] != NULL && strcmp(hash_table[index]->email, email) == 0) {
        return hash_table[index];
    }
    return NULL; // Participante não encontrado ou colisão não tratada para este email
}

/*
    A função original 'int buscaParticipante(Part *lista, char *email)'
    que você tinha no main.c era para buscar em uma *lista encadeada*,
    não na hash table global. Essa função é mais relevante para
    buscar na lista circular de participantes de um Evento.
    Vamos realocá-la para o módulo de inscrições onde ela será usada
    em 'insereParticipante' ao verificar se o participante já está inscrito
    em um evento específico.
*/

// int buscaParticipante(Part *lista, char *email) {
//     if (!lista) return 0;
//     Part *atual = lista;
//     do {
//         if (strcmp(atual->email, email) == 0) return 1;
//         atual = atual->prox;
//     } while (atual != lista);
//     return 0;
// }
