#ifndef COMUM_H
#define COMUM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Para _getch no Windows. Em Linux/macOS, pode precisar de ncurses ou termios.
// Por simplicidade, vamos manter conio.h aqui para compilar no Windows.
// Se estiver em Linux/macOS, comente a linha abaixo e use alternativas.
#ifdef _WIN32
#include <conio.h> 
#endif

// Cores para o console (ANSI Escape Codes)
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

// Tamanhos máximos e valores de sistema
#define MAX 50
#define MAX_EVENTOS 100
#define HASH_SIZE 100
#define ADMIN_PASS "admin123"
#define VAGAS_POR_EVENTO 3

// --- Estruturas de Dados ---
typedef struct Participante {
    int id;
    char nome[MAX];
    char email[MAX];
    struct Participante *prox;
} Part;

typedef struct Inscricao {
    int evento_id;
    int participante_id;
    char nome_participante[MAX];
    char email_participante[MAX];
    struct Inscricao *prox;
} Inscricao;

typedef struct Evento {
    int id;
    int vagas;
    int max_vagas;
    char nome[MAX];
    char categoria[MAX];
    Part *participantes;
    struct Evento *prox;
} Evento;

typedef struct FilaEspera {
    Part *participante;
    int evento_id;
    struct FilaEspera *prox;
} FilaEspera;

typedef struct PilhaEventos {
    Evento *evento;
    struct PilhaEventos *prox;
} PilhaEventos;

// --- Variáveis globais ---
extern Evento *eventos;
extern Inscricao *todas_inscricoes;
extern FilaEspera *inicio_fila, *fim_fila;
extern Part *hash_table[HASH_SIZE];
extern PilhaEventos *topo_pilha;
extern int next_evento_id;
extern int next_part_id;
extern int total_eventos;
extern Evento **indice_nome;
extern int total_indice;

// Protótipos de funções essenciais (opcional)
void inicializarHashParticipantes();
bool validarEmail(const char *email);

#endif // COMUM_H