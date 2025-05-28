#include "Utils/utils.h"
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Implementação da função para validar o formato de um email
int validarEmail(char *email) {
    int temArroba = 0, temPonto = 0, posArroba = -1;
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') {
            if (temArroba) return 0;
            temArroba = 1;
            posArroba = i;
        }
        if (email[i] == '.' && temArroba && i > posArroba + 1) temPonto = 1;
    }
    return temArroba && temPonto && posArroba > 0;
}

// Implementação da função para autenticar o administrador
int autenticarAdmin(char *senha) {
    return strcmp(senha, ADMIN_PASS) == 0;
}
