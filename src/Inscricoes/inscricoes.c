#include "FilaPilha/filaPilha.h" // Inclui os protótipos do próprio módulo
#include "Evento/eventos.h"    // Necessário para acessar a variável global 'eventos' na função listarFilaEspera

int buscaParticipanteEmEvento(Part *lista_participantes_evento, char *email)
{
    if (!lista_participantes_evento || !email)
        return 0;

    Part *atual = lista_participantes_evento;
    int count = 0;
    const int MAX_ITERATIONS = 1000;

    do
    {
        if (strcmp(atual->email, email) == 0)
            return 1;
        atual = atual->prox;
        count++;
    } while (atual != lista_participantes_evento && count < MAX_ITERATIONS);

    return 0;
}

// Funcao para verificar se ja existe uma inscricao global
int jaInscrito(int evento_id, int participante_id)
{
    if (evento_id <= 0 || participante_id <= 0)
        return 0;

    Inscricao *atual = todas_inscricoes;
    while (atual != NULL)
    {
        if (atual->evento_id == evento_id && atual->participante_id == participante_id)
        {
            return 1;
        }
        atual = atual->prox;
    }
    return 0;
}

void insereParticipante(Evento *e, Part *p) {
    if (!e || !p) {
        printf(RED "Erro: Evento ou participante inválido!\n" RESET);
        return;
    }
CLS

    // Verifica se já está inscrito (por e-mail)
    if (buscaParticipanteEmEvento(e->participantes, p->email)) {
        printf(RED "Você já está inscrito neste evento!\n" RESET);
        return;
    }

    if (e->vagas <= 0) {
        printf(YELLOW "Evento lotado! %s (%s) adicionado à fila de espera.\n" RESET,
               p->nome, p->email);
        insereFilaEspera(e, p);
        return;
    }

    // Adiciona à lista circular do evento
    if (!e->participantes) {
        e->participantes = p;
        p->prox = p;
    } else {
        p->prox = e->participantes->prox;
        e->participantes->prox = p;
        e->participantes = p;
    }

    e->vagas--;  // Garantia de decremento
    printf(GREEN "%s (%s) adicionado ao evento %s! Vagas restantes: %d/%d\n" RESET,
           p->nome, p->email, e->nome, e->vagas, e->max_vagas);

    // Registro global (mantido do seu código)
    Inscricao *nova_insc = malloc(sizeof(Inscricao));
    if (!nova_insc) {
        printf(RED "Erro ao registrar inscrição!\n" RESET);
        return;
    }
    nova_insc->evento_id = e->id;
    nova_insc->participante_id = p->id;
    strncpy(nova_insc->nome_participante, p->nome, MAX - 1);
    strncpy(nova_insc->email_participante, p->email, MAX - 1);
    nova_insc->prox = todas_inscricoes;
    todas_inscricoes = nova_insc;
}

    // Atualiza vagas (FALTAVA ESSA LINHA!)
    e->vagas--;

    // Registra a inscrição global
    Inscricao *nova_insc = malloc(sizeof(Inscricao));
    if (!nova_insc)
    {
        printf(RED "Erro ao registrar inscrição!\n" RESET);
        return;
    }
    nova_insc->evento_id = e->id;
    nova_insc->participante_id = p->id;
    strncpy(nova_insc->nome_participante, p->nome, MAX - 1);
    strncpy(nova_insc->email_participante, p->email, MAX - 1);
    nova_insc->prox = todas_inscricoes;
    todas_inscricoes = nova_insc;

    printf(GREEN "%s (%s) adicionado ao evento %s! Vagas restantes: %d/%d\n" RESET,
           p->nome, p->email, e->nome, e->vagas, e->max_vagas);
}

// Funcao para efetuar remocao de inscricao
int efetuarRemocaoInscricao(Evento *evento_alvo, const char *email_participante)
{
    if (!evento_alvo || !email_participante)
    {
        printf(RED "Erro: Parametros invalidos na remocao.\n" RESET);
        return 0;
    }

    if (!evento_alvo->participantes)
    {
        printf(YELLOW "Nenhum participante inscrito no evento %s.\n" RESET, evento_alvo->nome);
        return 0;
    }

    Part *participante_a_remover = NULL;
    Part *atual_p = evento_alvo->participantes;
    Part *anterior_p = NULL;
    int encontrado = 0;

    do
    {
        if (strcmp(atual_p->email, email_participante) == 0)
        {
            participante_a_remover = atual_p;
            encontrado = 1;
            break;
        }
        anterior_p = atual_p;
        atual_p = atual_p->prox;
    } while (atual_p != evento_alvo->participantes);

    if (!encontrado)
    {
        printf(RED "Participante %s nao encontrado no evento %s.\n" RESET,
               email_participante, evento_alvo->nome);
        return 0;
    }

    // Remove da lista circular
    if (anterior_p)
    {
        anterior_p->prox = participante_a_remover->prox;
        if (evento_alvo->participantes == participante_a_remover)
        {
            evento_alvo->participantes = anterior_p;
        }
    }
    else
    {
        if (participante_a_remover->prox == participante_a_remover)
        {
            evento_alvo->participantes = NULL;
        }
        else
        {
            Part *ultimo = evento_alvo->participantes;
            while (ultimo->prox != evento_alvo->participantes)
            {
                ultimo = ultimo->prox;
            }
            ultimo->prox = participante_a_remover->prox;
            evento_alvo->participantes = participante_a_remover->prox;
        }
    }

    evento_alvo->vagas++;
    printf(GREEN "Participante %s removido do evento %s.\n" RESET,
           email_participante, evento_alvo->nome);

    // Remove da lista global de inscricoes
    Inscricao *atual_i = todas_inscricoes;
    Inscricao *anterior_i = NULL;
    while (atual_i != NULL)
    {
        if (atual_i->evento_id == evento_alvo->id &&
            strcmp(atual_i->email_participante, email_participante) == 0)
        {

            if (anterior_i)
            {
                anterior_i->prox = atual_i->prox;
            }
            else
            {
                todas_inscricoes = atual_i->prox;
            }

            free(atual_i);
            break;
        }
        anterior_i = atual_i;
        atual_i = atual_i->prox;
    }

    // Verifica fila de espera
    FilaEspera *atual_f = inicio_fila;
    FilaEspera *anterior_f = NULL;
    while (atual_f != NULL)
    {
        if (atual_f->evento_id == evento_alvo->id)
        {
            Part *participante_promovido = atual_f->participante;

            if (anterior_f)
            {
                anterior_f->prox = atual_f->prox;
            }
            else
            {
                inicio_fila = atual_f->prox;
            }

            if (fim_fila == atual_f)
            {
                fim_fila = anterior_f;
            }

            printf(GREEN "Promovendo %s da fila de espera para o evento %s.\n" RESET,
                   participante_promovido->nome, evento_alvo->nome);

            insereParticipante(evento_alvo, participante_promovido);
            free(atual_f);
            break;
        }
        anterior_f = atual_f;
        atual_f = atual_f->prox;
    }

    return 1;
}

// Funcao para usuario remover propria inscricao
void removerInscricaoUsuario(char *email_usuario)
{
    if (!email_usuario)
        return;

    if (!todas_inscricoes)
    {
        printf(RED "\nNenhuma inscricao para remover!\n" RESET);
        return;
    }

    printf(BLUE "\n=== SUAS INSCRIÇÕES ===\n" RESET);
    Inscricao *temp = todas_inscricoes;
    Evento *eventos_inscrito[MAX_EVENTOS];
    int count = 0;
    int opcoes[MAX_EVENTOS];
    int opcao = 1;

    while (temp != NULL && count < MAX_EVENTOS)
    {
        if (strcmp(temp->email_participante, email_usuario) == 0)
        {
            Evento *e = eventos;
            while (e && e->id != temp->evento_id)
                e = e->prox;
            if (e)
            {
                printf(CYAN "%d. %s (%s)\n" RESET, opcao, e->nome, e->categoria);
                eventos_inscrito[count] = e;
                opcoes[count] = opcao;
                count++;
                opcao++;
            }
        }
        temp = temp->prox;
    }

    if (count == 0)
    {
        printf(YELLOW "\nVoce nao possui inscricoes ativas.\n" RESET);
        return;
    }

    printf(CYAN "\nEscolha o evento para remover (0 para cancelar): " RESET);
    int escolha;
    scanf("%d", &escolha);
    limparBuffer();

    if (escolha == 0)
    {
        printf(YELLOW "Operacao cancelada.\n" RESET);
        return;
    }

    Evento *evento_selecionado = NULL;
    for (int i = 0; i < count; i++)
    {
        if (opcoes[i] == escolha)
        {
            evento_selecionado = eventos_inscrito[i];
            break;
        }
    }

    if (!evento_selecionado)
    {
        printf(RED "Opcao invalida.\n" RESET);
        return;
    }

    if (efetuarRemocaoInscricao(evento_selecionado, email_usuario))
    {
        printf(GREEN "\nInscricao removida com sucesso!\n" RESET);
    }
    else
    {
        printf(RED "\nFalha ao remover inscricao.\n" RESET);
    }
}

// Funcao para administrador remover inscricoes
void adminRemoverInscricaoEspecifica()
{
    if (!eventos)
    {
        printf(YELLOW "\nNenhum evento cadastrado.\n" RESET);
        return;
    }

    printf(BLUE "\n=== REMOVER INSCRICAO (ADMIN) ===\n" RESET);
    listarEventos();

    char nome_evento[MAX];
    printf("\nNome do evento: ");
    scanf(" %[^\n]", nome_evento);
    limparBuffer();

    Evento *evento = buscaIndexada(nome_evento);
    if (!evento)
    {
        evento = eventos;
        while (evento && strcmp(evento->nome, nome_evento) != 0)
        {
            evento = evento->prox;
        }
    }

    if (!evento)
    {
        printf(RED "Evento nao encontrado.\n" RESET);
        return;
    }

    printf(BLUE "\nParticipantes inscritos:\n" RESET);
    if (!evento->participantes)
    {
        printf(YELLOW "Nenhum participante inscrito.\n" RESET);
        return;
    }

    Part *p = evento->participantes;
    do
    {
        printf(CYAN "- %s (%s)\n" RESET, p->nome, p->email);
        p = p->prox;
    } while (p != evento->participantes);

    char email[MAX];
    printf("\nEmail do participante a remover: ");
    scanf(" %[^\n]", email);
    limparBuffer();

    if (efetuarRemocaoInscricao(evento, email))
    {
        printf(GREEN "Inscricao removida com sucesso.\n" RESET);
    }
    else
    {
        printf(RED "Falha ao remover inscricao.\n" RESET);
    }
}

// Funcao para listar todas as inscricoes
void listarTodasInscricoes()
{
    if (!todas_inscricoes)
    {
        printf(YELLOW "\nNenhuma inscricao no sistema.\n" RESET);
        return;
    }

    printf(MAGENTA "\n=== TODAS AS INSCRICOES ===\n" RESET);
    Inscricao *atual = todas_inscricoes;
    while (atual != NULL)
    {
        Evento *e = eventos;
        while (e && e->id != atual->evento_id)
            e = e->prox;

        if (e)
        {
            printf(CYAN "Evento: %-20s | Participante: %-20s (%s)\n" RESET,
                   e->nome, atual->nome_participante, atual->email_participante);
        }
        else
        {
            printf(RED "Evento ID:%d (nao encontrado) | Participante: %s\n" RESET,
                   atual->evento_id, atual->nome_participante);
        }
        atual = atual->prox;
    }
}

// Funcao para listar inscricoes de um usuario
void listarMinhasInscricoes(char *email_usuario)
{
    if (!email_usuario || !todas_inscricoes)
    {
        printf(YELLOW "\nNenhuma inscricao encontrada.\n" RESET);
        return;
    }

    printf(BLUE "\n=== MINHAS INSCRICOES ===\n" RESET);
    int count = 0;
    Inscricao *atual = todas_inscricoes;

    while (atual != NULL)
    {
        if (strcmp(atual->email_participante, email_usuario) == 0)
        {
            Evento *e = eventos;
            while (e && e->id != atual->evento_id)
                e = e->prox;

            if (e)
            {
                printf(CYAN "%d. %s (%s) - Vagas: %d/%d\n" RESET,
                       ++count, e->nome, e->categoria,
                       e->max_vagas - e->vagas, e->max_vagas);
            }
        }
        atual = atual->prox;
    }

    if (count == 0)
    {
        printf(YELLOW "Voce nao possui inscricoes ativas.\n" RESET);
    }
}