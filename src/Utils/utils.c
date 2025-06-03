#include "Utils/utils.h"

// Limpa o buffer de entrada
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Valida formato de email
bool validarEmail(const char *email) {
    if (!email || strlen(email) == 0) {
        return false;
    }

    // Verifica comprimento máximo
    if (strlen(email) >= MAX) {
        return false;
    }

    // Verifica presença de @ e .
    char *ptr_arroba = strchr(email, '@');
    if (!ptr_arroba || ptr_arroba == email) {
        return false; // Nenhum @ ou @ no início
    }

    char *ptr_ponto = strchr(ptr_arroba, '.');
    if (!ptr_ponto || ptr_ponto == ptr_arroba + 1) {
        return false; // Nenhum . ou . logo após @
    }

    // Verifica se há caracteres após o último ponto
    if (strlen(ptr_ponto) < 2) {
        return false;
    }

    return true;
}

// Autentica administrador
bool autenticarAdmin(const char *senha) {
    if (!senha) return false;
    return strcmp(senha, ADMIN_PASS) == 0;
}

void lerSenha(char *senha, int tamanho_max) {
    int i = 0;
    char ch;
    
    while ((ch = getch()) != '\r' && i < tamanho_max - 1) {
        if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (isprint(ch)) {
            senha[i++] = ch;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

// Função para capitalizar nomes (ex: "joão silva" -> "João Silva")
void capitalizarNome(char *nome) {
    if (!nome || strlen(nome) == 0) return;

    // Primeira letra maiúscula
    nome[0] = toupper(nome[0]);

    // Demais letras após espaços
    for (int i = 1; nome[i]; i++) {
        if (nome[i-1] == ' ') {
            nome[i] = toupper(nome[i]);
        } else {
            nome[i] = tolower(nome[i]);
        }
    }
}

// Remove espaços extras de uma string
void removerEspacosExtras(char *str) {
    if (!str) return;
    
    int i, j;
    for (i = 0, j = 0; str[i]; i++) {
        if (!isspace(str[i]) || (i > 0 && !isspace(str[i-1]))) {
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
    
    // Remove espaço final se houver
    if (j > 0 && isspace(str[j-1])) {
        str[j-1] = '\0';
    }
}

// Verifica se uma string contém apenas letras e espaços
bool contemApenasLetras(const char *str) {
    if (!str) return false;
    
    for (int i = 0; str[i]; i++) {
        if (!isalpha(str[i]) && !isspace(str[i])) {
            return false;
        }
    }
    return true;
}

// Função para obter entrada segura de strings
void obterStringSegura(char *buffer, int tamanho, const char *mensagem) {
    if (!buffer || tamanho <= 0) return;
    
    printf("%s", mensagem);
    if (fgets(buffer, tamanho, stdin) == NULL) {
        buffer[0] = '\0';
        return;
    }
    
    // Remove newline
    buffer[strcspn(buffer, "\n")] = '\0';
    
    // Limpa buffer se necessário
    if (strlen(buffer) == tamanho - 1) {
        limparBuffer();
    }
}