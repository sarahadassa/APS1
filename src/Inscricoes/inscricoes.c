// Inscricoes/inscricoes.c
#include "Inscricoes/inscricoes.h" // Inclui os protótipos do próprio módulo
#include "Evento/eventos.h"        // Necessário para acessar 'eventos' e 'buscaIndexada', 'listarEventos'

// A função original 'buscaParticipante' do main.c (que busca em uma lista circular)
// foi renomeada e movida para cá.
int buscaParticipanteEmEvento(Part *lista_participantes_evento, char *email) {
    if (!lista_participantes_evento) return 0;

    Part *atual = lista_participantes_evento->prox;

    do {
        if (strcmp(atual->email, email) == 0) return 1;
        atual = atual->prox;
    } while (atual != lista_participantes_evento->prox);

    return 0;
}

// Implementação da função para inserir um participante em um evento
void insereParticipante(Evento *e, Part *p) {
    if (!e || !p) {
        printf(RED "Erro: Evento ou participante invalido!\n" RESET);
        return;
    }

    // A função validarEmail já está disponível via "Inscricoes/inscricoes.h" que inclui "Utils/utils.h"
    if (!validarEmail(p->email)) {
        printf(RED "Email invalido!\n" RESET);
        free(p);
        return;
    }

    if (e->participantes && buscaParticipanteEmEvento(e->participantes, p->email)) {
        printf(RED "Este email (%s) ja esta inscrito neste evento (%s)!\n" RESET,
               p->email, e->nome);
        Part *participante_existente_na_hash = buscaParticipantePorEmailNaHash(p->email); // De participantes.h
        if (p != participante_existente_na_hash) {
            free(p);
        }
        return;
    }

    int index = hashParticipante(p->email); // De participantes.h
    Part *participante_real = NULL;

    if (hash_table[index] != NULL) {
        if (strcmp(hash_table[index]->email, p->email) == 0) {
            participante_real = hash_table[index];
            printf(YELLOW "Usando cadastro existente para %s (ID: %d)\n" RESET,
                   p->email, participante_real->id);
            if (p != participante_real) {
                free(p);
            }
        } else {
            printf(YELLOW "Colisao de hash detectada para %s. Participante existente na hash será substituido.\n" RESET, hash_table[index]->email);
            free(hash_table[index]);
            hash_table[index] = p;
            participante_real = p;
        }
    } else {
        hash_table[index] = p;
        participante_real = p;
    }

    if (participante_real->id == 0) {
        participante_real->id = next_part_id++;
    }

    if (e->vagas <= 0) {
        printf(YELLOW "Evento lotado! %s (%s) adicionado a fila de espera.\n" RESET,
               participante_real->nome, participante_real->email);
        insereFilaEspera(e, participante_real); // De filaPilha.h
        return;
    }

    if (!e->participantes) {
        e->participantes = participante_real;
        participante_real->prox = participante_real;
    } else {
        participante_real->prox = e->participantes->prox;
        e->participantes->prox = participante_real;
        e->participantes = participante_real;
    }
    e->vagas--;

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

    pushEvento(e); // De filaPilha.h
}

// Função central para efetuar a remoção de uma inscrição
int efetuarRemocaoInscricao(Evento *evento_alvo, const char *email_participante) {
    if (!evento_alvo || !email_participante) {
        printf(RED "Erro: Evento ou email do participante nulo na remocao.\n" RESET);
        return 0;
    }

    Part *participante_a_remover_ptr = NULL;

    if (evento_alvo->participantes) {
        Part *atual_p = evento_alvo->participantes->prox;
        Part *anterior_p = evento_alvo->participantes;

        int encontrado_no_evento = 0;
        do {
            if (strcmp(atual_p->email, email_participante) == 0) {
                participante_a_remover_ptr = atual_p;
                encontrado_no_evento = 1;
                break;
            }
            anterior_p = atual_p;
            atual_p = atual_p->prox;
        } while (atual_p != evento_alvo->participantes->prox);

        if (encontrado_no_evento) {
            if (anterior_p == participante_a_remover_ptr && participante_a_remover_ptr->prox == participante_a_remover_ptr) {
                evento_alvo->participantes = NULL;
            } else {
                anterior_p->prox = participante_a_remover_ptr->prox;
                if (evento_alvo->participantes == participante_a_remover_ptr) {
                    evento_alvo->participantes = anterior_p;
                }
            }
            evento_alvo->vagas++;
            printf(GREEN "Participante %s removido da lista de inscritos do evento %s.\n" RESET, email_participante, evento_alvo->nome);
        } else {
            printf(RED "Participante %s nao encontrado na lista de inscritos do evento %s.\n" RESET, email_participante, evento_alvo->nome);
            return 0;
        }
    } else {
        printf(YELLOW "Nenhum participante inscrito no evento %s para remover.\n" RESET, evento_alvo->nome);
        return 0;
    }

    Inscricao *atual_i = todas_inscricoes;
    Inscricao *anterior_i = NULL;
    int removido_global = 0;
    while (atual_i != NULL) {
        if (atual_i->evento_id == evento_alvo->id && strcmp(atual_i->email_participante, email_participante) == 0) {
            if (anterior_i == NULL) {
                todas_inscricoes = atual_i->prox;
            } else {
                anterior_i->prox = atual_i->prox;
            }
            printf(GREEN "Inscricao de %s no evento %s (ID Evento: %d) removida da lista global.\n" RESET, atual_i->nome_participante, evento_alvo->nome, evento_alvo->id);
            free(atual_i);
            removido_global = 1;
            break;
        }
        anterior_i = atual_i;
        atual_i = atual_i->prox;
    }
    if (!removido_global) {
         printf(RED "Erro: Inscricao de %s para o evento %s nao encontrada na lista global, embora estivesse na lista do evento.\n" RESET, email_participante, evento_alvo->nome);
    }

    FilaEspera *atual_f = inicio_fila;
    FilaEspera *anterior_f = NULL;
    while (atual_f != NULL) {
        if (atual_f->evento_id == evento_alvo->id) {
            printf(YELLOW "Vaga aberta no evento %s. Processando fila de espera...\n" RESET, evento_alvo->nome);

            Part *participante_promovido = atual_f->participante;

            if (anterior_f == NULL) {
                inicio_fila = atual_f->prox;
            } else {
                anterior_f->prox = atual_f->prox;
            }
            if (fim_fila == atual_f) {
                fim_fila = anterior_f;
            }
            FilaEspera *temp_f = atual_f;
            atual_f = atual_f->prox;

            printf(GREEN "Movendo %s (%s) da fila de espera para o evento %s.\n" RESET, participante_promovido->nome, participante_promovido->email, evento_alvo->nome);

            insereParticipante(evento_alvo, participante_promovido);
            
            free(temp_f);
            pushEvento(evento_alvo);
            break;
        } else {
             anterior_f = atual_f;
             atual_f = atual_f->prox;
        }
    }
    return 1;
}

// Permite ao usuário remover sua própria inscrição
void removerInscricaoUsuario(char *email_usuario) {
    if (!todas_inscricoes) {
        printf(RED "\nNenhuma inscricao para remover!\n" RESET);
        return;
    }

    printf(BLUE "\n=== SUAS INSCRICOES ===\n" RESET);
    Inscricao *temp_i = todas_inscricoes;
    Evento *evento_inscrito[MAX_EVENTOS];
    int count_inscricoes_usuario = 0;
    int opcoes_validas[MAX_EVENTOS];

    int opcao_display = 1;
    while (temp_i != NULL && count_inscricoes_usuario < MAX_EVENTOS) {
        if (strcmp(temp_i->email_participante, email_usuario) == 0) {
            Evento *e = eventos;
            while (e && e->id != temp_i->evento_id) e = e->prox;
            if (e) {
                printf(CYAN "%d. Evento: %s\n" RESET, opcao_display, e->nome);
                evento_inscrito[count_inscricoes_usuario] = e;
                opcoes_validas[count_inscricoes_usuario] = opcao_display;
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
    limparBuffer();

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
    limparBuffer();

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
        } while (p_atual != inicio_iter);
    }

    if (!tem_participantes) {
        printf(YELLOW "Nenhum participante inscrito neste evento para remover.\n" RESET);
        return;
    }

    printf("\nDigite o email do participante a ser removido do evento '%s': ", evento_alvo->nome);
    scanf(" %[^\n]", email_participante);
    limparBuffer();

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
