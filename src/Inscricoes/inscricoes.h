// inscricoes.h
#ifndef INSCRICOES_H
#define INSCRICOES_H

#include "comum.h"      // Para as structs e variáveis globais
#include "utils.h"      // Para validarEmail
#include "participantes.h" // Para hashParticipante e buscaParticipantePorEmailNaHash
#include "fila_pilha.h" // Para insereFilaEspera e pushEvento (ainda a ser criado)

// Protótipos das funções de gerenciamento de inscrições
void insereParticipante(Evento *e, Part *p); // Renomeada de "insereParticipante" para "insereParticipanteEmEvento" para clareza
int buscaParticipanteEmEvento(Part *lista_participantes_evento, char *email); // Função para buscar em lista circular do evento
int efetuarRemocaoInscricao(Evento *evento_alvo, const char *email_participante);
void removerInscricaoUsuario(char *email_usuario);
void adminRemoverInscricaoEspecifica();
void listarTodasInscricoes();

#endif // INSCRICOES_H
