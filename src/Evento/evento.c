#include "comum.h"
#include "Utils/utils.h"
#include "evento.h"
#include "Participantes/participantes.h"
#include "Inscricoes/inscricoes.h"
#include "FilaPilha/filaPilha.h"
#include "Persistencia/persist.h"
// Implementação da função para cadastrar um novo evento
void cadastrarEvento() {
    if (total_eventos >= MAX_EVENTOS) {
        printf(RED "\nLimite maximo de eventos atingido!\n" RESET);
        return;
    }

    char nome_evento[MAX];
    printf("\nNome do evento: ");
    scanf(" %[^\n]", nome_evento);
    limparBuffer();

    // Verifica se já existe um evento com o mesmo nome
    // Usa buscaIndexada para uma busca mais rápida (se o índice estiver atualizado)
    Evento *e_existente = buscaIndexada(nome_evento);
    if (!e_existente) { // Se não encontrou no índice, tenta na lista linear (fallback)
        e_existente = eventos;
        while (e_existente != NULL) {
            if (strcmp(e_existente->nome, nome_evento) == 0) {
                break;
            }
            e_existente = e_existente->prox;
        }
    }

    if (e_existente != NULL) {
        printf(RED "\nJa existe um evento com o nome '%s' cadastrado!\n" RESET, nome_evento);
        return;
    }

    Evento *novo = malloc(sizeof(Evento));
    if (!novo) {
        printf(RED "\nErro de alocacao!\n" RESET);
        return;
    }
    novo->id = next_evento_id++;
    novo->prox = NULL;
    novo->vagas = VAGAS_POR_EVENTO;
    novo->max_vagas = VAGAS_POR_EVENTO;
    novo->participantes = NULL;
    strcpy(novo->nome, nome_evento);

    printf("Categoria: ");
    scanf(" %[^\n]", novo->categoria);
    limparBuffer();

    if (!eventos) eventos = novo;
    else {
        Evento *atual = eventos;
        while (atual->prox) atual = atual->prox;
        atual->prox = novo;
    }
    total_eventos++;
    atualizarIndice(); // Atualiza o índice após adicionar um evento
    bubbleSort();       // Reordena a lista de eventos (opcional, pode ser chamado ao listar)
    printf(GREEN "\nEvento %s cadastrado com sucesso!\n" RESET, novo->nome);
}

// Implementação da função para listar todos os eventos
void listarEventos() {
    printf("\n" BLUE "===================================\n");
    printf("        LISTA DE EVENTOS         \n");
    printf("===================================\n" RESET);

    if (!eventos) {
        printf("\n" YELLOW "Nenhum evento cadastrado ainda!\n" RESET);
        printf("\n" BLUE "-----------------------------------\n" RESET);
        return;
    }

    for (Evento *e = eventos; e; e = e->prox) {
        printf("\n> Evento: %s\n", e->nome);
        printf("> Categoria: %s\n", e->categoria);
        int num_participantes = e->max_vagas - e->vagas;
        printf("> Vagas ocupadas: %d/%d\n", num_participantes, e->max_vagas);
        printf("\n" BLUE "-----------------------------------\n" RESET);
    }
}

// Implementação do Bubble Sort para eventos
void bubbleSort() {
    if (!eventos || !eventos->prox) return;
    int trocado;
    Evento **ptr, *temp, *ultimo = NULL;
    do {
        trocado = 0;
        ptr = &eventos;
        while ((*ptr)->prox != ultimo) {
            int cmp_cat = strcmp((*ptr)->categoria, (*ptr)->prox->categoria);
            int cmp_nome = strcmp((*ptr)->nome, (*ptr)->prox->nome);
            if (cmp_cat > 0 || (cmp_cat == 0 && cmp_nome > 0)) {
                // Realiza a troca dos nós
                temp = *ptr;
                *ptr = (*ptr)->prox;
                temp->prox = (*ptr)->prox;
                (*ptr)->prox = temp;
                trocado = 1;
            }
            ptr = &(*ptr)->prox;
        }
        ultimo = *ptr; // Atualiza o último nó já "ordenado"
    } while (trocado);
}

// Implementação da função merge para o Merge Sort
Evento* merge(Evento *a, Evento *b) {
    if (!a) return b;
    if (!b) return a;

    if (strcmp(a->nome, b->nome) < 0) {
        a->prox = merge(a->prox, b);
        return a;
    } else {
        b->prox = merge(a, b->prox);
        return b;
    }
}

// Implementação da função split para o Merge Sort
void split(Evento *source, Evento **front, Evento **back) {
    Evento *slow = source;
    Evento *fast = source->prox;

    // Fast avança duas vezes, slow avança uma.
    // Quando fast chega ao fim, slow estará no meio.
    while (fast != NULL) {
        fast = fast->prox;
        if (fast != NULL) {
            slow = slow->prox;
            fast = fast->prox;
        }
    }

    *front = source;
    *back = slow->prox;
    slow->prox = NULL; // Quebra a lista ao meio
}

// Implementação do Merge Sort para eventos
void mergeSort(Evento **headRef) {
    Evento *head = *headRef;
    Evento *a, *b;

    // Caso base: 0 ou 1 elemento na lista
    if ((head == NULL) || (head->prox == NULL)) {
        return;
    }

    // Divide a lista em duas sublistas 'a' e 'b'
    split(head, &a, &b);

    // Ordena recursivamente as sublistas
    mergeSort(&a);
    mergeSort(&b);

    // Une as duas sublistas ordenadas
    *headRef = merge(a, b);
}


// Implementação da função para atualizar o índice de busca por nome
void atualizarIndice() {
    // Libera a memória do índice antigo se ele existir
    if (indice_nome) {
        free(indice_nome);
        indice_nome = NULL; // Importante para evitar dangling pointer
    }

    // Aloca um novo array para o índice com base no número total de eventos
    // total_eventos é uma variável global que deve ser mantida atualizada
    if (total_eventos == 0) {
        total_indice = 0;
        return;
    }
    indice_nome = malloc(total_eventos * sizeof(Evento*));
    if (!indice_nome) {
        printf(RED "Erro ao alocar memoria para o indice de eventos!\n" RESET);
        total_indice = 0; // Garante que o índice será tratado como vazio
        return;
    }

    // Preenche o índice com ponteiros para os eventos na lista encadeada
    Evento *e = eventos;
    for (int i = 0; i < total_eventos && e; i++) {
        indice_nome[i] = e;
        e = e->prox;
    }
    total_indice = total_eventos; // Atualiza o tamanho do índice

    // Ordena o array de ponteiros por nome do evento (usando Bubble Sort aqui para simplicidade,
    // mas Quick Sort ou Merge Sort seriam mais eficientes para arrays grandes)
    for (int i = 0; i < total_indice - 1; i++) {
        for (int j = 0; j < total_indice - i - 1; j++) {
            if (strcmp(indice_nome[j]->nome, indice_nome[j+1]->nome) > 0) {
                Evento *temp = indice_nome[j];
                indice_nome[j] = indice_nome[j+1];
                indice_nome[j+1] = temp;
            }
        }
    }
}

// Implementação da busca binária no índice de eventos
Evento* buscaIndexada(char *nome) {
    if (!indice_nome || total_indice == 0) return NULL;

    int inicio = 0;
    int fim = total_indice - 1;

    while (inicio <= fim) {
        int meio = inicio + (fim - inicio) / 2; // Evita overflow para (inicio + fim) muito grandes
        int cmp = strcmp(nome, indice_nome[meio]->nome);

        if (cmp == 0) return indice_nome[meio]; // Encontrado
        if (cmp < 0) fim = meio - 1;             // Nome está na primeira metade
        else inicio = meio + 1;                 // Nome está na segunda metade
    }
    return NULL; // Não encontrado
}
