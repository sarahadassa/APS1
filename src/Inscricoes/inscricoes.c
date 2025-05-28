#include "comum.h"
#include "Utils/utils.h"
#include "Evento/evento.h"
#include "Participantes/participantes.h"
#include "inscricoes.h"
#include "FilaPilha/filaPilha.h"
#include "Persistencia/persist.h"
// A função original 'buscaParticipante' do main.c (que busca em uma lista circular)
// foi renomeada e movida para cá.
int buscaParticipanteEmEvento(Part *lista_participantes_evento, char *email) {
    if (!lista_participantes_evento) return 0; // Lista vazia

    Part *atual = lista_participantes_evento->prox; // Começa pelo "head" da lista circular

    // Percorre a lista circular até voltar ao "head"
    do {
        if (strcmp(atual->email, email) == 0) return 1; // Participante encontrado
        atual = atual->prox;
    } while (atual != lista_participantes_evento->prox);

    return 0; // Participante não encontrado
}


// Implementação da função para inserir um participante em um evento
void insereParticipante(Evento *e, Part *p) {
    if (!e || !p) {
        printf(RED "Erro: Evento ou participante invalido!\n" RESET);
        return;
    }

    if (!validarEmail(p->email)) { // Usa a função de utils.h
        printf(RED "Email invalido!\n" RESET);
        free(p); // Libera o participante recém-alocado se o email for inválido
        return;
    }

    // Verifica se o participante já está inscrito neste evento (usando a função local)
    if (e->participantes && buscaParticipanteEmEvento(e->participantes, p->email)) {
        printf(RED "Este email (%s) ja esta inscrito neste evento (%s)!\n" RESET,
               p->email, e->nome);
        // Se o participante temporário não é o que já está na hash_table, o liberamos.
        // Cuidado para não liberar um ponteiro que já está sendo usado (por exemplo, vindo da hash_table)
        // Isso assume que 'p' pode ser um novo malloc ou um existente.
        Part *participante_existente_na_hash = buscaParticipantePorEmailNaHash(p->email);
        if (p != participante_existente_na_hash) {
            free(p);
        }
        return;
    }

    // Gerencia a hash table de participantes
    int index = hashParticipante(p->email); // Usa a função de participantes.h
    Part *participante_real = NULL;

    if (hash_table[index] != NULL) {
        // Colisão ou participante já existe na hash.
        // Seu código original tinha uma lógica de sobrescrever em caso de colisão,
        // o que não é ideal para uma hash. Por simplicidade, vou manter a sua lógica
        // que verifica o email para decidir se é o mesmo ou uma colisão.
        if (strcmp(hash_table[index]->email, p->email) == 0) {
            participante_real = hash_table[index];
            printf(YELLOW "Usando cadastro existente para %s (ID: %d)\n" RESET,
                   p->email, participante_real->id);
            if (p != participante_real) { // Se o 'p' passado era um novo malloc, libera-o
                free(p);
            }
        } else {
            // Colisão não tratada adequadamente (sobrescrevendo ou notificando)
            printf(YELLOW "Colisao de hash detectada para %s. Participante existente na hash será substituido.\n" RESET, hash_table[index]->email);
            // IMPORTANTE: Idealmente, você usaria encadeamento separado aqui (lista na posição do hash)
            // Para manter a lógica original (sobrescrevendo), libere o antigo e insira o novo.
            free(hash_table[index]); // Libera o participante anterior na posição
            hash_table[index] = p;
            participante_real = p;
        }
    } else {
        // Novo participante na hash table
        hash_table[index] = p;
        participante_real = p;
    }

    // Se o participante foi criado agora, atribui um ID
    if (participante_real->id == 0) { // Assume 0 se não foi atribuído ainda
        participante_real->id = next_part_id++;
    }


    // Verifica se há vagas no evento
    if (e->vagas <= 0) {
        printf(YELLOW "Evento lotado! %s (%s) adicionado a fila de espera.\n" RESET,
               participante_real->nome, participante_real->email);
        insereFilaEspera(e, participante_real); // Usa a função de fila_pilha.h
        return;
    }

    // Insere na lista circular de participantes do evento
    if (!e->participantes) {
        e->participantes = participante_real;
        participante_real->prox = participante_real; // Lista circular com um elemento
    } else {
        participante_real->prox = e->participantes->prox;
        e->participantes->prox = participante_real;
        e->participantes = participante_real; // O 'tail' aponta para o novo elemento
    }
    e->vagas--; // Diminui o número de vagas disponíveis

    // Registra a inscrição na lista global de todas as inscrições
    Inscricao *nova_insc = malloc(sizeof(Inscricao));
    if (!nova_insc) {
        printf(RED "Erro ao registrar inscricao!\n" RESET);
        return;
    }
    nova_insc->evento_id = e->id;
    nova_insc->participante_id = participante_real->id;
    strcpy(nova_insc->nome_participante, participante_real->nome);
    strcpy(nova_insc->email_participante, participante_real->email);
    nova_insc->prox = todas_inscricoes;
    todas_inscricoes = nova_insc;

    printf(GREEN "%s (%s) adicionado ao evento %s!\n" RESET,
           participante_real->nome, participante_real->email, e->nome);

    pushEvento(e); // Adiciona o evento à pilha de histórico (usa função de fila_pilha.h)
}

// Função central para efetuar a remoção de uma inscrição
int efetuarRemocaoInscricao(Evento *evento_alvo, const char *email_participante) {
    if (!evento_alvo || !email_participante) {
        printf(RED "Erro: Evento ou email do participante nulo na remocao.\n" RESET);
        return 0;
    }

    Part *participante_a_remover_ptr = NULL;

    // 1. Remover da lista circular de participantes do evento (evento_alvo->participantes)
    if (evento_alvo->participantes) {
        Part *atual_p = evento_alvo->participantes->prox; // "Head" da lista circular
        Part *anterior_p = evento_alvo->participantes;   // "Tail" da lista circular

        int encontrado_no_evento = 0;
        // Percorre a lista circular para encontrar o participante
        do {
            if (strcmp(atual_p->email, email_participante) == 0) {
                participante_a_remover_ptr = atual_p;
                encontrado_no_evento = 1;
                break;
            }
            anterior_p = atual_p;
            atual_p = atual_p->prox;
        } while (atual_p != evento_alvo->participantes->prox); // Condição de parada: voltou ao head

        if (encontrado_no_evento) {
            if (anterior_p == participante_a_remover_ptr && participante_a_remover_ptr->prox == participante_a_remover_ptr) {
                // Único participante na lista, a lista fica vazia
                evento_alvo->participantes = NULL;
            } else {
                anterior_p->prox = participante_a_remover_ptr->prox;
                // Se o removido era o "tail" (evento_alvo->participantes), atualiza o tail
                if (evento_alvo->participantes == participante_a_remover_ptr) {
                    evento_alvo->participantes = anterior_p;
                }
            }
            evento_alvo->vagas++; // Aumenta o número de vagas
            printf(GREEN "Participante %s removido da lista de inscritos do evento %s.\n" RESET, email_participante, evento_alvo->nome);
        } else {
            printf(RED "Participante %s nao encontrado na lista de inscritos do evento %s.\n" RESET, email_participante, evento_alvo->nome);
            return 0; // Participante não estava no evento.
        }
    } else {
        printf(YELLOW "Nenhum participante inscrito no evento %s para remover.\n" RESET, evento_alvo->nome);
        return 0;
    }

    // 2. Remover da lista global de inscrições (todas_inscricoes)
    Inscricao *atual_i = todas_inscricoes;
    Inscricao *anterior_i = NULL;
    int removido_global = 0;
    while (atual_i != NULL) {
        if (atual_i->evento_id == evento_alvo->id && strcmp(atual_i->email_participante, email_participante) == 0) {
            if (anterior_i == NULL) { // Removendo o primeiro da lista
                todas_inscricoes = atual_i->prox;
            } else {
                anterior_i->prox = atual_i->prox;
            }
            printf(GREEN "Inscricao de %s no evento %s (ID Evento: %d) removida da lista global.\n" RESET, atual_i->nome_participante, evento_alvo->nome, evento_alvo->id);
            free(atual_i); // Libera a memória da struct Inscricao
            removido_global = 1;
            break;
        }
        anterior_i = atual_i;
        atual_i = atual_i->prox;
    }
    if (!removido_global) {
         printf(RED "Erro: Inscricao de %s para o evento %s nao encontrada na lista global, embora estivesse na lista do evento.\n" RESET, email_participante, evento_alvo->nome);
    }

    // 3. Verifica a fila de espera para preencher a vaga aberta
    FilaEspera *atual_f = inicio_fila;
    FilaEspera *anterior_f = NULL;
    // Percorre a fila buscando por inscricoes para o evento que teve vaga aberta
    while (atual_f != NULL) {
        if (atual_f->evento_id == evento_alvo->id) { // Encontrou alguém na fila para este evento
            printf(YELLOW "Vaga aberta no evento %s. Processando fila de espera...\n" RESET, evento_alvo->nome);

            Part *participante_promovido = atual_f->participante;

            // Remover da fila de espera
            if (anterior_f == NULL) { // Primeiro da fila
                inicio_fila = atual_f->prox;
            } else {
                anterior_f->prox = atual_f->prox;
            }
            if (fim_fila == atual_f) { // Se era o último
                fim_fila = anterior_f;
            }
            FilaEspera *temp_f = atual_f; // Salva o nó para liberar
            atual_f = atual_f->prox;      // Avança para o próximo (para continuar o loop, se houver mais na fila)

            printf(GREEN "Movendo %s (%s) da fila de espera para o evento %s.\n" RESET, participante_promovido->nome, participante_promovido->email, evento_alvo->nome);

            // Tenta inserir o participante da fila no evento (ele já tem um ID, nome e email)
            // Passamos o participante *promovido* diretamente.
            // A função insereParticipante fará as verificações de vagas, etc.
            // Aqui, como acabamos de abrir uma vaga, ele deve ser inscrito.
            insereParticipante(evento_alvo, participante_promovido); // Esta chamada irá registrar a nova inscrição globalmente e diminuir a vaga novamente

            free(temp_f); // Libera a memória do nó da fila de espera
            pushEvento(evento_alvo); // Registra a ação na pilha
            break; // Apenas um participante é promovido por vez
        } else {
             anterior_f = atual_f;
             atual_f = atual_f->prox;
        }
    }
    return 1; // Remoção efetuada com sucesso (ou ao menos tentada)
}

// Permite ao usuário remover sua própria inscrição
void removerInscricaoUsuario(char *email_usuario) {
    if (!todas_inscricoes) {
        printf(RED "\nNenhuma inscricao para remover!\n" RESET);
        return;
    }

    printf(BLUE "\n=== SUAS INSCRICOES ===\n" RESET);
    Inscricao *temp_i = todas_inscricoes;
    Evento *evento_inscrito[MAX_EVENTOS]; // Array temporário para guardar os eventos do usuário
    int count_inscricoes_usuario = 0;
    int opcoes_validas[MAX_EVENTOS]; // Array para mapear opções do menu para índices reais

    int opcao_display = 1;
    while (temp_i != NULL && count_inscricoes_usuario < MAX_EVENTOS) {
        if (strcmp(temp_i->email_participante, email_usuario) == 0) {
            // Busca o evento pelo ID (precisa da lista global de eventos)
            Evento *e = eventos; // 'eventos' é uma variável global de comum.h
            while (e && e->id != temp_i->evento_id) e = e->prox;
            if (e) {
                printf(CYAN "%d. Evento: %s\n" RESET, opcao_display, e->nome);
                evento_inscrito[count_inscricoes_usuario] = e; // Guarda o ponteiro para o evento
                opcoes_validas[count_inscricoes_usuario] = opcao_display; // Guarda o número da opção
                count_inscricoes_usuario++;
                opcao_display++;
            }
        }
        temp_i = temp_i->prox;
    }

    if (count_inscricoes_usuario == 0) {
        printf(YELLOW "\nVoce nao possui inscricoes ativas!\n" RESET);
        return;
    }

    int escolha_usuario;
    printf(CYAN "\nDigite o numero do evento do qual deseja remover a sua inscricao (0 para cancelar): " RESET);
    scanf("%d", &escolha_usuario);
    limparBuffer(); // Usa a função de utils.h

    if (escolha_usuario == 0) {
        printf(YELLOW "Operacao cancelada.\n" RESET);
        return;
    }

    Evento *evento_selecionado = NULL;
    for(int i = 0; i < count_inscricoes_usuario; i++) {
        if (opcoes_validas[i] == escolha_usuario) {
            evento_selecionado = evento_inscrito[i];
            break;
        }
    }

    if (!evento_selecionado) {
        printf(RED "Escolha invalida.\n" RESET);
        return;
    }

    printf(YELLOW "\nRemovendo sua inscricao do evento: %s...\n" RESET, evento_selecionado->nome);
    if (efetuarRemocaoInscricao(evento_selecionado, email_usuario)) {
        printf(GREEN "\nSua inscricao no evento %s foi removida com sucesso!\n" RESET, evento_selecionado->nome);
    } else {
        printf(RED "\nFalha ao remover sua inscricao do evento %s.\n" RESET, evento_selecionado->nome);
    }
}

// Permite ao administrador remover uma inscrição específica
void adminRemoverInscricaoEspecifica() {
    char nome_evento[MAX];
    char email_participante[MAX];

    if (!eventos) {
        printf(YELLOW "\nNenhum evento cadastrado para remover inscricoes.\n" RESET);
        return;
    }
    if (!todas_inscricoes) {
        printf(YELLOW "\nNenhuma inscricao realizada no sistema.\n" RESET);
        return;
    }

    printf("\n" BLUE "--- REMOVER INSCRICAO ESPECIFICA (ADMIN) ---\n" RESET);
    listarEventos(); // Usa a função de eventos.h
    printf("\nDigite o nome do evento: ");
    scanf(" %[^\n]", nome_evento);
    limparBuffer(); // Usa a função de utils.h

    Evento *evento_alvo = buscaIndexada(nome_evento); // Tenta usar busca indexada
    if (!evento_alvo) {
        // Se busca indexada falhar, tenta a busca linear (fallback)
        evento_alvo = eventos;
        while(evento_alvo != NULL && strcmp(evento_alvo->nome, nome_evento) != 0) {
            evento_alvo = evento_alvo->prox;
        }
    }

    if (!evento_alvo) {
        printf(RED "Evento '%s' nao encontrado.\n" RESET, nome_evento);
        return;
    }

    printf(BLUE "\nParticipantes inscritos no evento '%s':\n" RESET, evento_alvo->nome);
    int tem_participantes = 0;
    if (evento_alvo->participantes) {
        Part *p_atual = evento_alvo->participantes->prox; // Começa pelo "head" da lista circular
        Part *inicio_iter = evento_alvo->participantes->prox; // Ponto de parada

        do {
            printf(CYAN "- %s (%s)\n" RESET, p_atual->nome, p_atual->email);
            tem_participantes = 1;
            p_atual = p_atual->prox;
        } while (p_atual != inicio_iter); // Continua até voltar ao início
    }

    if (!tem_participantes) {
        printf(YELLOW "Nenhum participante inscrito neste evento para remover.\n" RESET);
        return;
    }

    printf("\nDigite o email do participante a ser removido do evento '%s': ", evento_alvo->nome);
    scanf(" %[^\n]", email_participante);
    limparBuffer(); // Usa a função de utils.h

    if (!validarEmail(email_participante)) { // Usa a função de utils.h
        printf(RED "Email '%s' invalido.\n" RESET, email_participante);
        return;
    }

    printf(YELLOW "Tentando remover %s do evento %s...\n" RESET, email_participante, evento_alvo->nome);
    if (efetuarRemocaoInscricao(evento_alvo, email_participante)) {
        printf(GREEN "Remocao administrativa de %s do evento %s concluida.\n" RESET, email_participante, evento_alvo->nome);
    } else {
        printf(RED "Falha na remocao administrativa de %s do evento %s.\n" RESET, email_participante, evento_alvo->nome);
    }
}

// Lista todas as inscrições globais no sistema
void listarTodasInscricoes() {
    if (!todas_inscricoes) {
        printf("\n" YELLOW "Nenhuma inscricao!\n" RESET);
        return;
    }
    printf("\n" MAGENTA "=== INSCRICOES ===\n" RESET);
    for (Inscricao *i = todas_inscricoes; i; i = i->prox) {
        // Encontra o evento pelo ID (necessário percorrer a lista de eventos)
        Evento *e = eventos; // 'eventos' é uma variável global de comum.h
        while (e && e->id != i->evento_id) e = e->prox;
        if (e) {
            printf(CYAN "Evento: %s | Participante: %s (%s)\n" RESET, e->nome, i->nome_participante, i->email_participante);
        } else {
             printf(CYAN "Evento (ID: %d - NAO ENCONTRADO) | Participante: %s (%s)\n" RESET, i->evento_id, i->nome_participante, i->email_participante);
        }
    }
}
