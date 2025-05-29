#include "Persistencia/persist.h" // Inclui o cabeçalho do próprio módulo
#include "Evento/eventos.h"   
// --- Funções de Salvamento ---

void salvarEventosCSV() {
    FILE *file = fopen("eventos.csv", "w");
    if (!file) {
        printf(RED "Erro ao salvar eventos!\n" RESET);
        return;
    }
    fprintf(file, "ID,Nome,Categoria,Vagas,MaxVagas\n");
    for (Evento *e = eventos; e != NULL; e = e->prox) {
        fprintf(file, "%d,%s,%s,%d,%d\n", e->id, e->nome, e->categoria, e->vagas, e->max_vagas);
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

// --- Funções de Carregamento ---

void carregarEventosCSV() {
    FILE *file = fopen("eventos.csv", "r");
    if (!file) return;

    char linha[MAX * 4 + 20];
    fgets(linha, sizeof(linha), file); // Ignora o cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Evento *novo = malloc(sizeof(Evento));
        if (!novo) continue;
        novo->participantes = NULL; // Importante inicializar
        novo->prox = NULL;

        linha[strcspn(linha, "\n")] = 0; // Remove o newline

        // Adaptação para o caso de o CSV não ter max_vagas no passado
        int campos_lidos = sscanf(linha, "%d,%49[^,],%49[^,],%d,%d",
                                  &novo->id, novo->nome, novo->categoria, &novo->vagas, &novo->max_vagas);
        if (campos_lidos < 4) {
            // printf(RED "Erro ao ler linha de evento: %s\n" RESET, linha);
            free(novo);
            continue;
        }
        if (campos_lidos == 4) { // Se max_vagas não foi lido, usa o padrão
            novo->max_vagas = VAGAS_POR_EVENTO;
        }

        // Verifica se o evento já existe (para evitar duplicação no carregamento)
        // Uma busca linear simples é suficiente aqui pois a lista de eventos ainda está sendo montada
        Evento *existente = eventos;
        while (existente && existente->id != novo->id) existente = existente->prox;
        if (existente) {
            free(novo); // Evento com este ID já existe, não adiciona
            continue;
        }

        // Adiciona o novo evento ao final da lista de eventos
        if (eventos == NULL) eventos = novo;
        else {
            Evento *atual = eventos;
            while (atual->prox != NULL) atual = atual->prox;
            atual->prox = novo;
        }
        // Atualiza next_evento_id para ser maior que qualquer ID existente
        if (novo->id >= next_evento_id) next_evento_id = novo->id + 1;
        total_eventos++;
    }
    fclose(file);
}

void carregarParticipantesCSV() {
    FILE *file = fopen("participantes.csv", "r");
    if (!file) return;

    char linha[MAX * 3 + 10];
    fgets(linha, sizeof(linha), file); // Ignora o cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Part *novo = malloc(sizeof(Part));
        if (!novo) continue;
        novo->prox = NULL; // Inicializa, importante para a hash table

        linha[strcspn(linha, "\n")] = 0; // Remove o newline

        if (sscanf(linha, "%d,%49[^,],%49s", &novo->id, novo->nome, novo->email) != 3) {
             printf(RED "Erro ao ler linha de participante: %s\n" RESET, linha);
             free(novo);
             continue;
        }

        int index = hashParticipante(novo->email); // Usa função de participantes.h
        // Lógica de tratamento de colisão simples: sobrescreve se email for diferente
        // ou libera se email for o mesmo (já carregado)
        if (hash_table[index] == NULL) {
            hash_table[index] = novo;
            if (novo->id >= next_part_id) next_part_id = novo->id + 1;
        } else {
             if (strcmp(hash_table[index]->email, novo->email) == 0) {
                free(novo); // Participante com o mesmo email já carregado
            } else {
                printf(YELLOW "Colisao de hash nao tratada para email %s (carregamento). Participante nao carregado do CSV para evitar sobrescrita.\n" RESET, novo->email);
                free(novo);
             }
        }
    }
    fclose(file);
}

void carregarInscricoesCSV() {
    FILE *file = fopen("inscricoes.csv", "r");
    if (!file) return;

    char linha[MAX * 4 + 20];
    fgets(linha, sizeof(linha), file); // Ignora o cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        Inscricao *nova = malloc(sizeof(Inscricao));
        if (!nova) continue;

        linha[strcspn(linha, "\n")] = 0; // Remove o newline

        if (sscanf(linha, "%d,%d,%49[^,],%49s", &nova->evento_id, &nova->participante_id, nova->nome_participante, nova->email_participante) != 4) {
            printf(RED "Erro ao ler linha de inscricao: %s\n" RESET, linha);
            free(nova);
            continue;
        }

        // Adiciona a inscrição à lista global de inscrições
        nova->prox = todas_inscricoes;
        todas_inscricoes = nova;

        // Tenta associar o participante ao evento no carregamento
        Evento *e = eventos;
        while(e && e->id != nova->evento_id) e = e->prox;

        Part *p = NULL;
        // Busca o participante na hash table para obter o ponteiro real
        // Usa buscaParticipantePorEmailNaHash de participantes.h
        p = buscaParticipantePorEmailNaHash(nova->email_participante);


        if(e && p){
            // Verifica se o participante já está na lista de participantes do evento
            int ja_no_evento = 0;
            if(e->participantes){
                Part * atual_part_evento = e->participantes->prox; // Começa do head
                do {
                    if(atual_part_evento == p) { // Compara ponteiros para ver se já é o mesmo participante
                        ja_no_evento = 1;
                        break;
                    }
                    if (atual_part_evento == e->participantes) break; // Chegou ao "tail" sem encontrar
                    atual_part_evento = atual_part_evento->prox;
                } while (e->participantes != NULL && atual_part_evento != e->participantes->prox); // Chegou ao início da lista circular novamente
            }

            // Se o participante não está no evento, adiciona-o
            if(!ja_no_evento){
                 if (!e->participantes) {
                    e->participantes = p;
                    p->prox = p; // Lista circular com um elemento
                } else {
                    p->prox = e->participantes->prox;
                    e->participantes->prox = p;
                    e->participantes = p; // Atualiza o "tail"
                }
                e->vagas--; // Decrementa a vaga, pois o participante foi adicionado
            }
        }
    }
    fclose(file);
}

void carregarFilaEsperaCSV() {
    FILE *file = fopen("fila_espera.csv", "r");
    if (!file) return;

    char linha[MAX + 10];
    fgets(linha, sizeof(linha), file); // Ignora o cabeçalho

    while (fgets(linha, sizeof(linha), file)) {
        int evento_id_f, part_id_f;
        linha[strcspn(linha, "\n")] = 0; // Remove o newline

        if (sscanf(linha, "%d,%d", &evento_id_f, &part_id_f) != 2) {
            printf(RED "Erro ao ler linha da fila de espera: %s\n" RESET, linha);
            continue;
        }

        // Encontra o participante pelo ID na hash table
        Part *p = NULL;
        for (int k = 0; k < HASH_SIZE; k++) {
            if (hash_table[k] && hash_table[k]->id == part_id_f) {
                p = hash_table[k];
                break;
            }
        }
        if (!p) {
            printf(YELLOW "Participante ID %d da fila de espera nao encontrado na hash. Pode ter sido removido ou nao carregado.\n" RESET, part_id_f);
            continue;
        }

        // Encontra o evento pelo ID
        Evento *e_check = eventos;
        while(e_check && e_check->id != evento_id_f) e_check = e_check->prox;
        if(!e_check){
            printf(YELLOW "Evento ID %d da fila de espera nao encontrado. Pode ter sido removido ou nao carregado.\n" RESET, evento_id_f);
            continue;
        }

        // Cria o nó da fila de espera
        FilaEspera *nova_f = malloc(sizeof(FilaEspera));
        if (!nova_f) continue;

        nova_f->evento_id = evento_id_f;
        nova_f->participante = p; // Usa o ponteiro para o participante existente
        nova_f->prox = NULL;

        // Adiciona à fila de espera
        if (!inicio_fila) inicio_fila = fim_fila = nova_f;
        else { fim_fila->prox = nova_f;
        fim_fila = nova_f; }
    }
    fclose(file);
}

void carregarTudoCSV() {
    // A ordem de carregamento é crucial: Eventos, Participantes, Inscrições, Fila de Espera
    // Eventos e Participantes precisam existir antes de serem referenciados por Inscrições e Fila.
    carregarEventosCSV();
    carregarParticipantesCSV();
    carregarInscricoesCSV();
    carregarFilaEsperaCSV();
    atualizarIndice(); // Atualiza o índice de eventos após o carregamento
}

// Implementação da função para liberar toda a memória alocada
void liberarMemoria() {
    // Libera a lista de eventos e seus participantes
    Evento *e = eventos;
    while (e != NULL) {
        Evento *temp_e = e;
        e = e->prox;

        if (temp_e->participantes != NULL) {

            temp_e->participantes->prox = NULL;
        }
        free(temp_e); // Libera o nó do evento
    }
    eventos = NULL;

    // Libera a lista global de inscrições
    Inscricao *i = todas_inscricoes;
    while (i != NULL) {
        Inscricao *temp_i = i;
        i = i->prox;
        free(temp_i);
    }
    todas_inscricoes = NULL;

    // Libera a fila de espera
    FilaEspera *f = inicio_fila;
    while (f != NULL) {
        FilaEspera *temp_f = f;
        f = f->prox;
        free(temp_f); // Libera o nó da fila. O participante em 'temp_f->participante'
                      // é um ponteiro para um participante da hash_table, então não é liberado aqui.
    }
    inicio_fila = fim_fila = NULL;

    // Libera os participantes na hash table
    for (int k = 0; k < HASH_SIZE; k++) {
        if (hash_table[k] != NULL) {
            free(hash_table[k]); // Libera o Participante
            hash_table[k] = NULL;
        }
    }

    // Libera a pilha de eventos
    while (topo_pilha) {
        PilhaEventos *temp_pilha = topo_pilha;
        topo_pilha = topo_pilha->prox;
        free(temp_pilha); // Libera o nó da pilha. O evento em 'temp_pilha->evento'
                          // é um ponteiro para um evento da lista global de eventos, então não é liberado aqui.
    }
    topo_pilha = NULL;

    // Libera o índice de busca
    if (indice_nome) {
        free(indice_nome);
        indice_nome = NULL;
    }
    total_indice = 0;

    // Reseta IDs e contadores para um estado inicial limpo (útil se o programa continuar)
    total_eventos = 0;
    next_evento_id = 1;
    next_part_id = 1;
}
