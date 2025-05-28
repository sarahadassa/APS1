// utils.h
#ifndef UTILS_H
#define UTILS_H

#include "comum.h" // Inclui as definições e constantes globais

// Protótipos das funções utilitárias
void limparBuffer();
int validarEmail(char *email);
int autenticarAdmin(char *senha); // Mover para cá se quiser

#endif // UTILS_H
