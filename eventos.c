#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define MAX 50
#define MAX_PART 3
#define MAX_EVENTOS 100
#define HASH_SIZE 100
#define ADMIN_PASS "admin123"

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

typedef struct Participante { 
    int id; 
    char nome[MAX]; 
    char email[MAX]; 
    struct Participante *prox; 
} Part;

typedef struct Inscricao { 
    int evento_id, participante_id; 
    char nome_participante[MAX]; 
    char email_participante[MAX]; 
    struct Inscricao *prox; 
} Inscricao;

typedef struct Evento { 
    int id, vagas; 
    char nome[MAX], categoria[MAX]; 
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

Evento *eventos = NULL; 
Inscricao *todas_inscricoes = NULL; 
FilaEspera *inicio_fila = NULL, *fim_fila = NULL;
Part *hash_table[HASH_SIZE] = {NULL}; 
PilhaEventos *topo_pilha = NULL; 
int next_evento_id = 1, next_part_id = 1, total_eventos = 0;

Evento **indice_nome = NULL;
int total_indice = 0;

void bubbleSort(), mergeSort(Evento**), insereParticipante(Evento*, Part*), insereFilaEspera(Evento*, Part*), liberarMemoria();
Evento* merge(Evento*, Evento*); 
int hashParticipante(char*), buscaParticipante(Part*, char*);
void split(Evento*, Evento**, Evento**), listarEventos(), listarTodasInscricoes(), listarFilaEspera();
void atualizarIndice(), menuUsuario(), menuAdmin(), removerInscricaoUsuario(char*);
Evento* buscaIndexada(char *nome);
int autenticarAdmin(char *senha);

int validarEmail(char *email) {
    int temArroba = 0, temPonto = 0;
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') temArroba = 1;
        if (email[i] == '.' && temArroba) temPonto = 1;
    }
    return temArroba && temPonto;
}

void pushEvento(Evento *e) {
    PilhaEventos *novo = malloc(sizeof(PilhaEventos));
    if (!novo) { printf(RED "Erro ao alocar memoria!\n" RESET); return; }
    novo->evento = e;
    novo->prox = topo_pilha;
    topo_pilha = novo;
}

Evento* popEvento() {
    if (!topo_pilha) return NULL;
    PilhaEventos *temp = topo_pilha;
    Evento *e = temp->evento;
    topo_pilha = topo_pilha->prox;
    free(temp);
    return e;
}

void listarPilhaEventos() {
    if (!topo_pilha) {
        printf(YELLOW "\nPilha de eventos vazia!\n" RESET);
        return;
    }
    printf(MAGENTA "\n=== PILHA DE EVENTOS (HISTORICO) ===\n" RESET);
    PilhaEventos *atual = topo_pilha;
    while (atual) {
        printf(CYAN "Evento: %s (ID: %d)\n" RESET, atual->evento->nome, atual->evento->id);
        atual = atual->prox;
    }
}

void bubbleSort() {
    if (!eventos || !eventos->prox) return;
    int trocado; Evento **ptr, *temp, *ultimo = NULL;
    do {
        trocado = 0; ptr = &eventos;
        while ((*ptr)->prox != ultimo) {
            int cmp_cat = strcmp((*ptr)->categoria, (*ptr)->prox->categoria);
            int cmp_nome = strcmp((*ptr)->nome, (*ptr)->prox->nome);
            if (cmp_cat > 0 || (cmp_cat == 0 && cmp_nome > 0)) {
                temp = *ptr; *ptr = (*ptr)->prox; temp->prox = (*ptr)->prox; (*ptr)->prox = temp; trocado = 1;
            } ptr = &(*ptr)->prox;
        } ultimo = *ptr;
    } while (trocado);
}

Evento* merge(Evento *a, Evento *b) {
    if (!a || !b) return a ? a : b;
    if (strcmp(a->nome, b->nome) < 0) { a->prox = merge(a->prox, b); return a; }
    else { b->prox = merge(a, b->prox); return b; }
}

void split(Evento *source, Evento **front, Evento **back) {
    Evento *slow = source, *fast = source->prox;
    while (fast && fast->prox) { slow = slow->prox; fast = fast->prox->prox; }
    *front = source; *back = slow->prox; slow->prox = NULL;
}

void mergeSort(Evento **head) {
    if (*head == NULL || (*head)->prox == NULL) return;
    Evento *a, *b; split(*head, &a, &b); mergeSort(&a); mergeSort(&b); *head = merge(a, b);
}

int hashParticipante(char *email) { 
    int sum = 0; 
    for (int i = 0; email[i]; sum += email[i++]); 
    return sum % HASH_SIZE; 
}

int buscaParticipante(Part *lista, char *email) {
    if (!lista) return 0;
    Part *atual = lista;
    do { 
        if (strcmp(atual->email, email) == 0) return 1; 
        atual = atual->prox; 
    } while (atual != lista);
    return 0;
}

void atualizarIndice() {
    if (indice_nome) free(indice_nome);
    indice_nome = malloc(total_eventos * sizeof(Evento*));
    
    Evento *e = eventos;
    for (int i = 0; i < total_eventos && e; i++) {
        indice_nome[i] = e;
        e = e->prox;
    }
    
    for (int i = 0; i < total_eventos-1; i++) {
        for (int j = 0; j < total_eventos-i-1; j++) {
            if (strcmp(indice_nome[j]->nome, indice_nome[j+1]->nome) > 0) {
                Evento *temp = indice_nome[j];
                indice_nome[j] = indice_nome[j+1];
                indice_nome[j+1] = temp;
            }
        }
    }
}

Evento* buscaIndexada(char *nome) {
    int inicio = 0, fim = total_eventos - 1;
    while (inicio <= fim) {
        int meio = (inicio + fim) / 2;
        int cmp = strcmp(nome, indice_nome[meio]->nome);
        if (cmp == 0) return indice_nome[meio];
        if (cmp < 0) fim = meio - 1;
        else inicio = meio + 1;
    }
    return NULL;
}

void insereFilaEspera(Evento *e, Part *p) {
    FilaEspera *nova = malloc(sizeof(FilaEspera));
    if (!nova) { printf(RED "Erro ao alocar memoria!\n" RESET); return; }
    nova->evento_id = e->id; 
    nova->participante = p; 
    nova->prox = NULL;
    
    if (!inicio_fila) inicio_fila = fim_fila = nova;
    else { fim_fila->prox = nova; fim_fila = nova; }
    printf(YELLOW "Participante %s na fila do evento %s.\n" RESET, p->nome, e->nome);
}

void insereParticipante(Evento *e, Part *p) {
    if (!validarEmail(p->email)) { 
        printf(RED "Email inválido!\n" RESET); 
        free(p); 
        return; 
    }
    
    // Verifica se o email já está inscrito NESTE EVENTO específico
    if (e->participantes) {
        Part *atual = e->participantes;
        do {
            if (strcmp(atual->email, p->email) == 0) {
                printf(RED "Este email já está inscrito neste evento!\n" RESET);
                free(p);
                return;
            }
            atual = atual->prox;
        } while (atual != e->participantes);
    }
    
    // Verifica se o participante já existe no sistema
    int index = hashParticipante(p->email);
    if (hash_table[index] != NULL) {
        // Se já existe, usa o participante existente
        printf(YELLOW "Usando cadastro existente para %s\n" RESET, p->email);
        free(p); // Libera o novo que foi criado
        p = hash_table[index]; // Usa o existente
    } else {
        // Se não existe, adiciona à tabela hash
        hash_table[index] = p;
    }
    
    if (e->vagas <= 0) { 
        printf(YELLOW "Evento lotado! %s na fila.\n" RESET, p->nome); 
        insereFilaEspera(e, p); 
        return; 
    }
    
    // Insere no evento
    if (!e->participantes) { 
        e->participantes = p; 
        p->prox = p; 
    } else { 
        p->prox = e->participantes->prox; 
        e->participantes->prox = p; 
        e->participantes = p; 
    }
    e->vagas--;
    
    // Registra a inscrição
    Inscricao *nova = malloc(sizeof(Inscricao));
    if (!nova) { 
        printf(RED "Erro ao registrar inscricao!\n" RESET); 
        return; 
    }
    nova->evento_id = e->id; 
    nova->participante_id = p->id; 
    strcpy(nova->nome_participante, p->nome);
    strcpy(nova->email_participante, p->email);
    nova->prox = todas_inscricoes; 
    todas_inscricoes = nova;
    
    printf(GREEN "%s (%s) adicionado ao evento %s!\n" RESET, p->nome, p->email, e->nome);
}

void removerInscricaoUsuario(char *email_usuario) {
    if (!todas_inscricoes) {
        printf(RED "\nNenhuma inscricao para remover!\n" RESET);
        return;
    }

    Inscricao *ultima = NULL;
    Inscricao *atual = todas_inscricoes;
    
    while (atual) {
        if (strcmp(atual->email_participante, email_usuario) == 0) {
            ultima = atual;
        }
        atual = atual->prox;
    }

    if (!ultima) {
        printf(RED "\nVoce nao possui inscricoes!\n" RESET);
        return;
    }

    Evento *e = eventos;
    while (e && e->id != ultima->evento_id) e = e->prox;
    
    if (e) {
        printf(YELLOW "\nRemovendo sua inscricao no evento: %s\n" RESET, e->nome);
        
        e->vagas++;

        Part *part_atual = e->participantes;
        Part *anterior = NULL;
        int encontrado = 0;

        if (part_atual) {
            do {
                if (strcmp(part_atual->email, ultima->email_participante) == 0) {
                    encontrado = 1;
                    if (anterior) {
                        anterior->prox = part_atual->prox;
                        if (part_atual == e->participantes) {
                            e->participantes = (anterior == part_atual) ? NULL : anterior;
                        }
                    } else {
                        if (part_atual->prox == part_atual) {
                            e->participantes = NULL;
                        } else {
                            Part *temp = part_atual;
                            while (temp->prox != part_atual) temp = temp->prox;
                            temp->prox = part_atual->prox;
                            e->participantes = temp;
                        }
                    }
                    free(part_atual);
                    break;
                }
                anterior = part_atual;
                part_atual = part_atual->prox;
            } while (part_atual != e->participantes && e->participantes != NULL);
        }

        int index = hashParticipante(ultima->email_participante);
        if (hash_table[index] && strcmp(hash_table[index]->email, ultima->email_participante) == 0) {
            free(hash_table[index]);
            hash_table[index] = NULL;
        }
    }

    if (ultima == todas_inscricoes) {
        todas_inscricoes = ultima->prox;
    } else {
        Inscricao *temp = todas_inscricoes;
        while (temp->prox != ultima) temp = temp->prox;
        temp->prox = ultima->prox;
    }

    printf(GREEN "Inscricao removida com sucesso!\n" RESET);
    free(ultima);
}

void listarFilaEspera() {
    if (!inicio_fila) { 
        printf("\n" YELLOW "Fila de espera vazia!\n" RESET); 
        return; 
    }
    printf("\n" MAGENTA "=== FILA DE ESPERA ===\n" RESET);
    FilaEspera *atual = inicio_fila;
    while (atual) {
        Evento *e = eventos;
        while (e && e->id != atual->evento_id) e = e->prox;
        if (e) printf(CYAN "Evento: %s | Participante: %s (%s)\n" RESET, e->nome, atual->participante->nome, atual->participante->email);
        atual = atual->prox;
    }
}

void listarEventos() {
    printf("\n" BLUE "===================================\n");
    printf("          LISTA DE EVENTOS          \n");
    printf("===================================\n" RESET);
    
    if (!eventos) {
        printf("\n" YELLOW "Nenhum evento cadastrado ainda!\n" RESET);
        printf("\n" BLUE "-----------------------------------\n" RESET);
        return;
    }
    
    for (Evento *e = eventos; e; e = e->prox) {
        printf("\n> Evento: %s\n", e->nome);
        printf("> Categoria: %s\n", e->categoria);
        printf("> Vagas: %d/%d\n", e->vagas, MAX_PART);
        
        if (e->participantes) {
            printf("> Participantes:\n");
            Part *p = e->participantes;
            int count = 1;
            do {
                printf("   %d. %s (%s)\n", count++, p->nome, p->email);
                p = p->prox;
            } while (p != e->participantes);
        } else {
            printf("> Nenhum participante inscrito ainda!\n");
        }
        printf("\n" BLUE "-----------------------------------\n" RESET);
    }
}

void listarTodasInscricoes() {
    if (!todas_inscricoes) { 
        printf("\n" YELLOW "Nenhuma inscricao!\n" RESET); 
        return; 
    }
    printf("\n" MAGENTA "=== INSCRICOES ===\n" RESET);
    for (Inscricao *i = todas_inscricoes; i; i = i->prox) {
        Evento *e = eventos;
        while (e && e->id != i->evento_id) e = e->prox;
        if (e) {
            printf(CYAN "Evento: %s | Participante: %s (%s)\n" RESET, e->nome, i->nome_participante, i->email_participante);
        }
    }
}

void liberarMemoria() {
    for (Evento *e = eventos, *temp_e; e; e = temp_e) {
        temp_e = e->prox;
        if (e->participantes) {
            Part *p = e->participantes->prox, *temp_p;
            while (p != e->participantes) { 
                temp_p = p; 
                p = p->prox; 
                free(temp_p); 
            }
            free(e->participantes);
        } 
        free(e);
    }
    
    for (Inscricao *i = todas_inscricoes, *temp_i; i; i = temp_i) { 
        temp_i = i->prox; 
        free(i); 
    }
    
    for (FilaEspera *f = inicio_fila, *temp_f; f; f = temp_f) { 
        temp_f = f->prox; 
        free(f->participante); 
        free(f); 
    }
    
    while (topo_pilha) {
        PilhaEventos *temp = topo_pilha;
        topo_pilha = topo_pilha->prox;
        free(temp);
    }
    
    free(indice_nome);
}

int autenticarAdmin(char *senha) {
    return strcmp(senha, ADMIN_PASS) == 0;
}

void salvarEventosCSV() {
    FILE *file = fopen("eventos.csv", "w");
    if (!file) {
        printf(RED "Erro ao salvar eventos!\n" RESET);
        return;
    }
    fprintf(file, "ID,Nome,Categoria,Vagas\n");
    for (Evento *e = eventos; e != NULL; e = e->prox) {
        fprintf(file, "%d,%s,%s,%d\n", e->id, e->nome, e->categoria, e->vagas);
    }
    fclose(file);
}

void salvarParticipantesCSV() {
    FILE *file = fopen("participantes.csv", "w");
    if (!file) {
        printf(RED "Erro ao salvar participantes!\n" RESET);
        return;
    }
    fprintf(file, "ID,Nome,Email\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        if (hash_table[i] != NULL) {
            fprintf(file, "%d,%s,%s\n", hash_table[i]->id, hash_table[i]->nome, hash_table[i]->email);
        }
    }
    fclose(file);
}

void salvarInscricoesCSV() {
    FILE *file = fopen("inscricoes.csv", "w");
    if (!file) {
        printf(RED "Erro ao salvar inscricoes!\n" RESET);
        return;
    }
    fprintf(file, "EventoID,ParticipanteID,Nome,Email\n");
    for (Inscricao *i = todas_inscricoes; i != NULL; i = i->prox) {
        fprintf(file, "%d,%d,%s,%s\n", i->evento_id, i->participante_id, i->nome_participante, i->email_participante);
    }
    fclose(file);
}

void salvarFilaEsperaCSV() {
    FILE *file = fopen("fila_espera.csv", "w");
    if (!file) {
        printf(RED "Erro ao salvar fila de espera!\n" RESET);
        return;
    }
    fprintf(file, "EventoID,ParticipanteID\n");
    for (FilaEspera *f = inicio_fila; f != NULL; f = f->prox) {
        fprintf(file, "%d,%d\n", f->evento_id, f->participante->id);
    }
    fclose(file);
}

void salvarTudoCSV() {
    salvarEventosCSV();
    salvarParticipantesCSV();
    salvarInscricoesCSV();
    salvarFilaEsperaCSV();
    printf(GREEN "Todos os dados foram salvos em arquivos CSV!\n" RESET);
}

void carregarEventosCSV() {
    FILE *file = fopen("eventos.csv", "r");
    if (!file) return;

    char linha[MAX * 3];
    fgets(linha, sizeof(linha), file); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Evento *novo = malloc(sizeof(Evento));
        sscanf(linha, "%d,%[^,],%[^,],%d", &novo->id, novo->nome, novo->categoria, &novo->vagas);
        novo->prox = NULL;
        novo->participantes = NULL;

        if (eventos == NULL) eventos = novo;
        else {
            Evento *atual = eventos;
            while (atual->prox != NULL) atual = atual->prox;
            atual->prox = novo;
        }
        if (novo->id >= next_evento_id) next_evento_id = novo->id + 1;
    }
    fclose(file);
}

void carregarParticipantesCSV() {
    FILE *file = fopen("participantes.csv", "r");
    if (!file) return;

    char linha[MAX * 3];
    fgets(linha, sizeof(linha), file); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Part *novo = malloc(sizeof(Part));
        sscanf(linha, "%d,%[^,],%s", &novo->id, novo->nome, novo->email);
        novo->prox = NULL;

        int index = hashParticipante(novo->email);
        hash_table[index] = novo;

        if (novo->id >= next_part_id) next_part_id = novo->id + 1;
    }
    fclose(file);
}

void carregarInscricoesCSV() {
    FILE *file = fopen("inscricoes.csv", "r");
    if (!file) return;

    char linha[MAX * 4];
    fgets(linha, sizeof(linha), file); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Inscricao *nova = malloc(sizeof(Inscricao));
        sscanf(linha, "%d,%d,%[^,],%s", &nova->evento_id, &nova->participante_id, nova->nome_participante, nova->email_participante);
        nova->prox = todas_inscricoes;
        todas_inscricoes = nova;
    }
    fclose(file);
}

void carregarFilaEsperaCSV() {
    FILE *file = fopen("fila_espera.csv", "r");
    if (!file) return;

    char linha[MAX];
    fgets(linha, sizeof(linha), file); // Pula cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        int evento_id, part_id;
        sscanf(linha, "%d,%d", &evento_id, &part_id);

        Part *p = NULL;
        for (int i = 0; i < HASH_SIZE; i++) {
            if (hash_table[i] && hash_table[i]->id == part_id) {
                p = hash_table[i];
                break;
            }
        }
        if (!p) continue;

        FilaEspera *nova = malloc(sizeof(FilaEspera));
        nova->evento_id = evento_id;
        nova->participante = p;
        nova->prox = NULL;

        if (!inicio_fila) inicio_fila = fim_fila = nova;
        else { fim_fila->prox = nova; fim_fila = nova; }
    }
    fclose(file);
}

void carregarTudoCSV() {
    carregarEventosCSV();
    carregarParticipantesCSV();
    carregarInscricoesCSV();
    carregarFilaEsperaCSV();
    printf(GREEN "Dados carregados dos arquivos CSV!\n" RESET);
}

void menuUsuario() {
    int op;
    char email_usuario[MAX];
    
    printf("\nDigite seu email para identificacao: ");
    scanf(" %[^\n]", email_usuario);
    
    while (!validarEmail(email_usuario)) {
        printf(RED "Email invalido! Digite novamente: " RESET);
        scanf(" %[^\n]", email_usuario);
    }

    char email_display[MAX];
    char *arroba = strchr(email_usuario, '@');
    if (arroba) {
        strncpy(email_display, email_usuario, arroba - email_usuario);
        email_display[arroba - email_usuario] = '\0';
    } else {
        strcpy(email_display, email_usuario);
    }
    
    while (1) {
        printf("\n" MAGENTA "=== MENU USUARIO (%s) ===\n" RESET, email_display);
        printf("1. Listar Eventos Disponiveis\n");
        printf("2. Inscrever-se em Evento\n");
        printf("3. Remover Minha Inscricao\n");
        printf("0. Voltar\n");
        printf(CYAN "Opcao: " RESET);
        scanf("%d", &op);
        
        switch (op) {
            case 1: 
                listarEventos(); 
                break;
            case 2: {
                if (!eventos) { 
                    printf("\n" YELLOW "Nenhum evento cadastrado ainda!\n" RESET); 
                    break; 
                }
                printf("\n" BLUE "=== EVENTOS DISPONIVEIS ===\n" RESET);
                for (Evento *e = eventos; e; e = e->prox) 
                    printf("Nome: %s - Categoria: %s - Vagas: %d\n", e->nome, e->categoria, e->vagas);
                
                char nome_evento[MAX];
                printf("\nNome do evento: "); 
                scanf(" %[^\n]", nome_evento);
                
                Evento *e = buscaIndexada(nome_evento);
                if (!e) { 
                    printf("\n" RED "Evento nao encontrado!\n" RESET); 
                    break; 
                }
                
                // Verifica se já está inscrito neste evento
                int jaInscrito = 0;
                if (e->participantes) {
                    Part *atual = e->participantes;
                    do {
                        if (strcmp(atual->email, email_usuario) == 0) {
                            printf(RED "\nVoce ja esta inscrito neste evento!\n" RESET);
                            jaInscrito = 1;
                            break;
                        }
                        atual = atual->prox;
                    } while (atual != e->participantes);
                }
                
                if (jaInscrito) {
                    break; // Volta direto para o menu se já estiver inscrito
                }
                
                Part *p = malloc(sizeof(Part));
                if (!p) { 
                    printf("\n" RED "Erro de alocacao!\n" RESET); 
                    break; 
                }
                p->id = next_part_id++; 
                strcpy(p->email, email_usuario);
                printf("Digite seu nome: ");
                scanf(" %[^\n]", p->nome);
                insereParticipante(e, p);
                break;
            }
            case 3: 
                removerInscricaoUsuario(email_usuario); 
                break;
            case 0: 
                return;
            default: 
                printf("\n" RED "Opcao invalida!\n" RESET);
        }
    }
}

void menuAdmin() {
    char senha[MAX];
    int i = 0;
    char ch;

    while (1) {
        printf("\nDigite a senha de administrador: ");
        i = 0;
        
        // Limpa o buffer do teclado
        while ((ch = _getch()) != '\r' && i < MAX-1) {
            if (ch == '\b') { // Tecla backspace
                if (i > 0) {
                    i--;
                    printf("\b \b"); // Apaga o asterisco
                }
            } else if (ch >= ' ' && ch <= '~') { // Caracteres visíveis
                senha[i++] = ch;
                printf("*");
            }
        }
        senha[i] = '\0';
        printf("\n");
        
        if (autenticarAdmin(senha)) {
            break; // Senha correta, sai do loop
        } else {
            printf(RED "\nSenha incorreta! Tente novamente.\n" RESET);
        }
    }

    int op;
    while (1) {
        printf("\n" MAGENTA "=== MENU ADMINISTRADOR ===\n" RESET);
        printf("1. Cadastrar Evento\n");
        printf("2. Listar Eventos\n");
        printf("3. Listar Todas Inscricoes\n");
        printf("4. Listar Fila de Espera\n");
        printf("5. Remover Ultima Inscricao\n");
        printf("6. Buscar Evento por Nome\n");
        printf("7. Listar Historico\n"); 
        printf("0. Voltar\n");
        printf(CYAN "Opcao: " RESET);
        scanf("%d", &op);
        
        switch (op) {
            case 1: {
                if (total_eventos >= MAX_EVENTOS) { 
                    printf("\n" RED "Limite maximo!\n" RESET); 
                    break; 
                }
                Evento *novo = malloc(sizeof(Evento));
                if (!novo) { 
                    printf("\n" RED "Erro de alocacao!\n" RESET); 
                    break; 
                }
                novo->id = next_evento_id++; 
                novo->prox = NULL; 
                novo->vagas = MAX_PART; 
                novo->participantes = NULL;
                printf("\nNome do evento: "); 
                scanf(" %[^\n]", novo->nome);
                printf("Categoria: "); 
                scanf(" %[^\n]", novo->categoria);
                
                if (!eventos) eventos = novo;
                else { 
                    Evento *atual = eventos; 
                    while (atual->prox) atual = atual->prox; 
                    atual->prox = novo; 
                }
                total_eventos++; 
                atualizarIndice();
                mergeSort(&eventos); 
                printf("\n" GREEN "Evento %s cadastrado com sucesso!\n" RESET, novo->nome);
                break;
            }
            case 2: 
                listarEventos(); 
                break;
            case 3: 
                listarTodasInscricoes(); 
                break;
            case 4: 
                listarFilaEspera(); 
                break;
            case 5: {
                if (!todas_inscricoes) {
                    printf(RED "\nNenhuma inscricao para remover!\n" RESET);
                    break;
                }
                Inscricao *ultima = todas_inscricoes;
                todas_inscricoes = todas_inscricoes->prox;
                
                Evento *e = eventos;
                while (e && e->id != ultima->evento_id) e = e->prox;
                
                if (e) {
                    e->vagas++;
                    pushEvento(e);
                    printf(GREEN "\nInscricao de %s (%s) removida com sucesso!\n" RESET, ultima->nome_participante, ultima->email_participante);
                }
                free(ultima);
                break;
            }
            case 6: {
                char nome_busca[MAX];
                printf("\nDigite o nome do evento: ");
                scanf(" %[^\n]", nome_busca);
                Evento *e = buscaIndexada(nome_busca);
                if (e) {
                    printf(GREEN "\nEvento encontrado:\n" RESET);
                    printf("Nome: %s\n", e->nome);
                    printf("Categoria: %s\n", e->categoria);
                    printf("Vagas disponiveis: %d/%d\n", e->vagas, MAX_PART);
                } else {
                    printf(RED "\nEvento nao encontrado!\n" RESET);
                }
                break;
            }
            case 7: 
                listarPilhaEventos();
                break;
            case 0: 
                return;
            default: 
                printf("\n" RED "Opcao invalida!\n" RESET);
        }
    }
}

int main() {
    carregarTudoCSV(); // Carrega dados ao iniciar

    int op;
    while (1) {
        printf("\n" CYAN "=== SISTEMA DE EVENTOS ===\n" RESET);
        printf("1. Sou Usuario\n");
        printf("2. Sou Administrador\n");
        printf("0. Sair\n");
        printf(CYAN "Opcao: " RESET);
        scanf("%d", &op);
        
        switch (op) {
            case 1: 
                menuUsuario(); 
                break;
            case 2: 
                menuAdmin(); 
                break;
            case 0: 
                salvarTudoCSV(); // Salva dados ao sair
                liberarMemoria(); 
                return 0;
            default: 
                printf("\n" RED "Opcao invalida!\n" RESET);
        }
    }
}