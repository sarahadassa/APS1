// Utils/utils.h
#ifndef UTILS_H
#define UTILS_H

#include "comum.h" // Este aqui precisa funcionar!

// Protótipos das funções
void limparBuffer();
bool validarEmail(const char *email);
bool autenticarAdmin(const char *senha);
void capitalizarNome(char *nome);
void removerEspacosExtras(char *str);
bool contemApenasLetras(const char *str);
void obterStringSegura(char *buffer, int tamanho, const char *mensagem);

#endif // UTILS_H