// comum.h
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
#define MAX 50 // Tamanho máximo para nomes, categorias, emails, etc.
#define MAX_EVENTOS 100 // Número máximo de eventos (se usarmos array estático para índice)
#define HASH_SIZE 100 // Tamanho da tabela hash de participantes
#define ADMIN_PASS "admin123" // Senha do administrador
#define VAGAS_POR_EVENTO 3 // Vagas padrão para novos eventos

// --- Definições das Estruturas de Dados ---

typedef struct Participante {
    int id;
    char nome[MAX];
    char email[MAX];
    struct Participante *prox; // Próximo na lista circular do evento
                               // ou próximo na lista de colisão da hash table
} Part;

typedef struct Inscricao {
    int evento_id;
    int participante_id;
    char nome_participante[MAX]; // Armazena nome e email para facilitar relatórios
    char email_participante[MAX];
    struct Inscricao *prox;
} Inscricao;

typedef struct Evento {
    int id;
    int vagas; // Vagas restantes
    int max_vagas; // Vagas totais
    char nome[MAX];
    char categoria[MAX];
    Part *participantes; // Lista circular de participantes inscritos no evento
    struct Evento *prox; // Próximo evento na lista de eventos
} Evento;

typedef struct FilaEspera {
    Part *participante; // Ponteiro para o participante na fila
    int evento_id;      // ID do evento para o qual o participante está esperando
    struct FilaEspera *prox;
} FilaEspera;

typedef struct PilhaEventos {
    Evento *evento; // Ponteiro para o evento na pilha
    struct PilhaEventos *prox;
} PilhaEventos;

// --- Variáveis globais (declaradas como extern, definidas em um .c) ---
// Usar extern aqui significa que a variável será definida em algum outro lugar.
// Isso evita múltiplos símbolos na linkagem.

extern Evento *eventos;
extern Inscricao *todas_inscricoes;
extern FilaEspera *inicio_fila, *fim_fila;
extern Part *hash_table[HASH_SIZE];
extern PilhaEventos *topo_pilha;
extern int next_evento_id;
extern int next_part_id;
extern int total_eventos;
extern Evento **indice_nome; // Para busca indexada
extern int total_indice;     // Tamanho atual do índice

#endif // COMUM_H
