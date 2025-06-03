#include "Participantes/participantes.h" // Inclui o cabeçalho do próprio módulo
#include "Utils/utils.h"                // Necessário para validarEmail e limparBuffer

// Inicialização da tabela hash
void inicializarHashParticipantes() {
    for (int i = 0; i < HASH_SIZE; i++) {
        hash_table[i] = NULL;
    }
}

// Função hash melhorada (djb2 algorithm)
int hashParticipante(const char *email) {
    if (!email) return 0;
    
    unsigned long hash = 5381;
    int c;
    while ((c = *email++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_SIZE;
}

// Busca participante na hash table (com normalização de email)
Part* buscaParticipantePorEmailNaHash(const char *email) {
    if (!email || strlen(email) == 0) return NULL;
    
    // Normaliza email para minúsculas
    char email_normalizado[MAX];
    strncpy(email_normalizado, email, MAX-1);
    email_normalizado[MAX-1] = '\0';
    
    for (int i = 0; email_normalizado[i]; i++) {
        email_normalizado[i] = tolower(email_normalizado[i]);
    }
    
    int index = hashParticipante(email_normalizado);
    Part *p = hash_table[index];
    
    while (p != NULL) {
        if (strcasecmp(p->email, email_normalizado) == 0) {
            return p;
        }
        p = p->prox;
    }
    return NULL;
}

// Obtém ou cria novo participante (versão robusta)
Part* obterOuCriarParticipante(const char *email_usuario) {
    // Validação rigorosa do email
    if (!email_usuario || strlen(email_usuario) == 0) {
        printf(RED "Email inválido: string vazia!\n" RESET);
        return NULL;
    }
    
    if (strlen(email_usuario) >= MAX) {
        printf(RED "Email inválido: muito longo (máx %d caracteres)!\n" RESET, MAX-1);
        return NULL;
    }
    
    if (!validarEmail(email_usuario)) {
        printf(RED "Email inválido: formato incorreto!\n" RESET);
        return NULL;
    }

    // Busca participante existente
    Part *p = buscaParticipantePorEmailNaHash(email_usuario);
    if (p != NULL) {
        printf(YELLOW "Usando cadastro existente para %s (ID: %d)\n" RESET, p->email, p->id);
        return p;
    }

    // Aloca memória para novo participante
    p = malloc(sizeof(Part));
    if (!p) {
        printf(RED "Erro de alocação para novo participante!\n" RESET);
        return NULL;
    }
    
    // Preenche dados do participante
    p->id = next_part_id++;
    
    // Armazena email em minúsculas
    strncpy(p->email, email_usuario, MAX-1);
    p->email[MAX-1] = '\0';
    for (int i = 0; p->email[i]; i++) {
        p->email[i] = tolower(p->email[i]);
    }
    
    // Solicita nome com tratamento seguro
    printf("Digite seu nome (máx %d caracteres): ", MAX-1);
    if (fgets(p->nome, MAX, stdin) == NULL) {
        printf(RED "Erro ao ler nome do participante!\n" RESET);
        free(p);
        return NULL;
    }
    p->nome[strcspn(p->nome, "\n")] = '\0'; // Remove quebra de linha
    
    // Valida nome
    if (strlen(p->nome) == 0) {
        printf(RED "Nome não pode ser vazio!\n" RESET);
        free(p);
        return NULL;
    }
    
    // Capitaliza nome (Primeira letra maiúscula)
    capitalizarNome(p->nome);
    
    // Insere na hash table
    int index = hashParticipante(p->email);
    p->prox = hash_table[index];
    hash_table[index] = p;
    
    printf(GREEN "Novo participante criado com sucesso! (ID: %d)\n" RESET, p->id);
    return p;
}