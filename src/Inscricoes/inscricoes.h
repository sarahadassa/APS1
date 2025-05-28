// inscricoes.h
#ifndef INSCRICOES_H
#define INSCRICOES_H

#include "comum.h"       // Para as structs e variáveis globais
#include "Utils/utils.h"      // Para validarEmail (usado em insereParticipante)
#include "Participantes/participantes.h" // Para hashParticipante e buscaParticipantePorEmailNaHash
#include "FilaPilha/filaPilha.h" // Para insereFilaEspera e pushEvento

// Protótipos das funções de gerenciamento de inscrições
void insereParticipante(Evento *e, Part *p);
int buscaParticipanteEmEvento(Part *lista_participantes_evento, char *email);
int efetuarRemocaoInscricao(Evento *evento_alvo, const char *email_participante);
void removerInscricaoUsuario(char *email_usuario);
void adminRemoverInscricaoEspecifica();
void listarTodasInscricoes();

#endif // INSCRICOES_H
