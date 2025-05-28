// Participantes/participantes.c
#include "Participantes/participantes.h" // Inclui o cabeçalho do próprio módulo
#include "Utils/utils.h"                // Necessário para validarEmail e limparBuffer

// Implementação da função de hash para emails de participantes
int hashParticipante(char *email) {
    int sum = 0;
    for (int i = 0; email[i]; sum += email[i++]);
    return sum % HASH_SIZE;
}

// Implementação da busca de participante na hash table por email
Part* buscaParticipantePorEmailNaHash(char *email) {
    if (!email || strlen(email) == 0) {
        return NULL;
    }
    int index = hashParticipante(email);
    if (hash_table[index] != NULL && strcmp(hash_table[index]->email, email) == 0) {
        return hash_table[index];
    }
    return NULL; // Participante não encontrado ou colisão não tratada para este email
}

// Implementação da função para obter um participante existente ou criar um novo
Part* obterOuCriarParticipante(char *email_usuario) {
    int index = hashParticipante(email_usuario);
    Part *p = NULL;

    // Tenta encontrar o participante na hash table
    if (hash_table[index] != NULL && strcmp(hash_table[index]->email, email_usuario) == 0) {
        p = hash_table[index];
        printf(YELLOW "Usando cadastro existente para %s (ID: %d)\n" RESET, p->email, p->id);
    } else {
        // Participante não encontrado na hash ou colisão não tratada, cria um novo
        p = malloc(sizeof(Part));
        if (!p) {
            printf(RED "Erro de alocacao para novo participante!\n" RESET);
            return NULL;
        }
        p->id = next_part_id++;
        strcpy(p->email, email_usuario);
        printf("Digite seu nome: ");
        scanf(" %[^\n]", p->nome);
        limparBuffer(); // Usa a função de utils.h
        p->prox = NULL; // Importante para a hash table ou lista circular

        // Adiciona/sobrescreve o novo participante na hash table
        if (hash_table[index] != NULL) {
            // Se houver uma colisão (email diferente), libere o antigo antes de sobrescrever
            // ATENÇÃO: Essa é uma política de colisão muito simples (sobrescrever).
            // Para um sistema mais robusto, você implementaria encadeamento separado (lista de colisões).
            printf(YELLOW "Colisao de hash detectada para %s. Participante anterior sera substituido.\n" RESET, hash_table[index]->email);
            free(hash_table[index]);
        }
        hash_table[index] = p;
    }
    return p;
}
