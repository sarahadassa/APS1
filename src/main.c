// main.c
#include "comum.h"
#include "Utils/utils.h"
#include "Evento/eventos.h" // Corrigido para "eventos.h" se for esse o nome do arquivo
#include "Participantes/participantes.h"
#include "Inscricoes/inscricoes.h"
#include "FilaPilha/filaPilha.h" // Corrigido para "filaPilha.h" se for esse o nome
#include "Persistencia/persist.h" // Corrigido para "persist.h" se for esse o nome

// Variáveis globais (DEFINIDAS aqui, declaradas como extern em comum.h)
Evento *eventos = NULL;
Inscricao *todas_inscricoes = NULL;
FilaEspera *inicio_fila = NULL, *fim_fila = NULL;
Part *hash_table[HASH_SIZE] = {NULL};
PilhaEventos *topo_pilha = NULL;
int next_evento_id = 1, next_part_id = 1, total_eventos = 0;
Evento **indice_nome = NULL; // Para busca indexada
int total_indice = 0; // Tamanho atual do índice

// Protótipos das funções que ficarão em main.c
void menuUsuario();
void menuAdmin();

// Implementação do menu de usuário
void menuUsuario() {
    int op;
    char email_usuario[MAX];

    printf("\nDigite seu email para identificacao: ");
    scanf(" %[^\n]", email_usuario);
    limparBuffer();

    while (!validarEmail(email_usuario)) {
        printf(RED "Email invalido! Digite novamente: " RESET);
        scanf(" %[^\n]", email_usuario);
        limparBuffer();
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
        if(scanf("%d", &op) != 1) {
            limparBuffer();
            printf(RED "\nEntrada invalida! Por favor, digite um numero.\n" RESET);
            continue;
        }
        limparBuffer();

        switch (op) {
            case 1:
                listarEventos(); // Chama a função do módulo Eventos
                break;
            case 2: {
                if (!eventos) {
                    printf("\n" YELLOW "Nenhum evento cadastrado ainda!\n" RESET);
                    break;
                }
                printf("\n" BLUE "=== EVENTOS DISPONIVEIS ===\n" RESET);
                listarEventos(); // Reutiliza a função de listar eventos

                char nome_evento[MAX];
                printf("\nNome do evento: ");
                scanf(" %[^\n]", nome_evento);
                limparBuffer();

                Evento *e = buscaIndexada(nome_evento); // Chama a função do módulo Eventos
                // Se a busca indexada falhou, tenta uma busca linear simples
                if (!e) {
                    e = eventos;
                    while(e != NULL && strcmp(e->nome, nome_evento) != 0) {
                        e = e->prox;
                    }
                }

                if (!e) {
                    printf("\n" RED "Evento nao encontrado!\n" RESET);
                    break;
                }

                // Verifica se o participante já está inscrito neste evento
                if (e->participantes && buscaParticipanteEmEvento(e->participantes, email_usuario)) {
                    printf(RED "\nVoce ja esta inscrito neste evento!\n" RESET);
                    break;
                }

                // Obtém ou cria o participante (nova função no módulo Participantes)
                Part *p = obterOuCriarParticipante(email_usuario);
                if (!p) {
                    printf(RED "Erro ao obter/criar participante!\n" RESET);
                    break;
                }

                insereParticipante(e, p); // Chama a função do módulo Inscricoes
                break;
            }
            case 3:
                removerInscricaoUsuario(email_usuario); // Chama a função do módulo Inscricoes
                break;
            case 0:
                return;
            default:
                printf(RED "Opcao invalida!\n" RESET);
        }
    }
}

// Implementação do menu de administrador
void menuAdmin() {
    char senha[MAX];
    int i = 0;
    char ch;

    int tentativas = 0;
    const int MAX_TENTATIVAS = 3;

    while (tentativas < MAX_TENTATIVAS) {
        printf("\nDigite a senha de administrador: ");
        i = 0;
        #ifdef _WIN32
        while ((ch = _getch()) != '\r' && i < MAX - 1) {
        #else // Para Linux/macOS
        // Para uma solução mais robusta em Linux/macOS, você precisaria de termios.h
        // e desabilitar o eco do terminal. Por simplicidade, usaremos getchar()
        // mas a senha será visível.
        while ((ch = getchar()) != '\n' && i < MAX - 1) {
            if (ch == 127 || ch == '\b') { // Backspace (ASCII 127 ou '\b')
                 if (i > 0) {
                    i--;
                    printf("\b \b");
                }
            } else {
                senha[i++] = ch;
                printf("*");
            }
        }
        #endif
            if (ch == '\b') {
                if (i > 0) {
                    i--;
                    printf("\b \b");
                }
            } else if (isprint(ch)) {
                senha[i++] = ch;
                printf("*");
            }
        }
        senha[i] = '\0';
        printf("\n");

        if (autenticarAdmin(senha)) { // Chama a função do módulo Utils
            break;
        } else {
            tentativas++;
            printf(RED "\nSenha incorreta! Tentativas restantes: %d\n" RESET, MAX_TENTATIVAS - tentativas);
            if (tentativas >= MAX_TENTATIVAS) {
                printf(RED "Numero maximo de tentativas atingido. Acesso negado.\n" RESET);
                return;
            }
        }
    }

    int op;
    while (1) {
        printf("\n" MAGENTA "=== MENU ADMINISTRADOR ===\n" RESET);
        printf("1. Cadastrar Evento\n");
        printf("2. Listar Eventos\n");
        printf("3. Listar Todas Inscricoes\n");
        printf("4. Listar Fila de Espera\n");
        printf("5. Remover Inscricao Especifica\n");
        printf("6. Buscar Evento por Nome\n");
        printf("7. Listar Historico\n");
        printf("0. Voltar\n");
        printf(CYAN "Opcao: " RESET);
        if(scanf("%d", &op) != 1) {
            limparBuffer();
            printf(RED "\nEntrada invalida! Por favor, digite um numero.\n" RESET);
            continue;
        }
        limparBuffer();

        switch (op) {
            case 1:
                cadastrarEvento(); // Chama a função do módulo Eventos
                break;
            case 2:
                listarEventos(); // Chama a função do módulo Eventos
                break;
            case 3:
                listarTodasInscricoes(); // Chama a função do módulo Inscricoes
                break;
            case 4:
                listarFilaEspera(); // Chama a função do módulo FilaPilha
                break;
            case 5: {
                adminRemoverInscricaoEspecifica(); // Chama a função do módulo Inscricoes
                break;
            }
            case 6: {
                char nome_busca[MAX];
                printf("\nDigite o nome do evento: ");
                scanf(" %[^\n]", nome_busca);
                limparBuffer();
                Evento *e = buscaIndexada(nome_busca); // Chama a função do módulo Eventos
                // A função buscaIndexada já deve ter a lógica de fallback se necessário
                if (e) {
                    printf(GREEN "\nEvento encontrado:\n" RESET);
                    printf("Nome: %s\n", e->nome);
                    printf("Categoria: %s\n", e->categoria);
                    printf("Vagas disponiveis: %d/%d\n", e->vagas, e->max_vagas);
                } else {
                    printf(RED "\nEvento nao encontrado!\n" RESET);
                }
                break;
            }
            case 7:
                listarPilhaEventos(); // Chama a função do módulo FilaPilha
                break;
            case 0:
                return;
            default:
                printf("\n" RED "Opcao invalida!\n" RESET);
        }
    }
}

// Função principal do programa
int main() {
    carregarTudoCSV(); // Chama a função do módulo Persistencia

    int op;
    while (1) {
        printf("\n" CYAN "=== SISTEMA DE EVENTOS ===\n" RESET);
        printf("1. Sou Usuario\n");
        printf("2. Sou Administrador\n");
        printf("0. Sair\n");
        printf(CYAN "Opcao: " RESET);
        if(scanf("%d", &op) != 1) {
            limparBuffer();
            printf(RED "\nEntrada invalida! Por favor, digite um numero.\n" RESET);
            continue;
        }
        limparBuffer();
        switch (op) {
            case 1:
                menuUsuario();
                break;
            case 2:
                menuAdmin();
                break;
            case 0:
                salvarTudoCSV(); // Chama a função do módulo Persistencia
                liberarMemoria(); // Chama a função do módulo Persistencia
                printf(GREEN "Sistema encerrado!\n" RESET);
                return 0;
            default:
                printf("\n" RED "Opcao invalida!\n" RESET);
        }
    }
}
