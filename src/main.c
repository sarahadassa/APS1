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

// Protótipos das funções locais
void menuUsuario();
void menuAdmin();

// Implementação do menu de usuário
void menuUsuario() {
    int op;
    char email_usuario[MAX];

    printf("\nDigite seu email para identificacao: ");
    obterStringSegura(email_usuario, MAX, "");
    
    while (!validarEmail(email_usuario)) {
        printf(RED "Email invalido! Digite novamente: " RESET);
        obterStringSegura(email_usuario, MAX, "");
    }

    // Exibe apenas a parte antes do @ no menu
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
        printf("4. Listar Minhas Inscricoes\n");
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
                listarEventos();
                break;
            case 2: {
                if (!eventos) {
                    printf("\n" YELLOW "Nenhum evento cadastrado ainda!\n" RESET);
                    break;
                }
                
                printf("\n" BLUE "=== EVENTOS DISPONIVEIS ===\n" RESET);
                listarEventos();

                char nome_evento[MAX];
                printf("\nNome do evento: ");
                obterStringSegura(nome_evento, MAX, "");

                Evento *e = buscaIndexada(nome_evento);
                if (!e) {
                    e = eventos;
                    while(e && strcmp(e->nome, nome_evento) != 0) {
                        e = e->prox;
                    }
                }

                if (!e) {
                    printf("\n" RED "Evento nao encontrado!\n" RESET);
                    break;
                }

                Part *p = obterOuCriarParticipante(email_usuario);
                if (p) {
                    insereParticipante(e, p);
                }
                break;
            }
            case 3:
                removerInscricaoUsuario(email_usuario);
                break;
            case 4:
                listarMinhasInscricoes(email_usuario);
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
    int tentativas = 0;
    const int MAX_TENTATIVAS = 3;

    while (tentativas < MAX_TENTATIVAS) {
        printf("\nDigite a senha de administrador: ");
        lerSenha(senha, MAX);  // Função segura para ler senha

        if (autenticarAdmin(senha)) {
            break;
        }
        
        tentativas++;
        printf(RED "\nSenha incorreta! Tentativas restantes: %d\n" RESET, 
              MAX_TENTATIVAS - tentativas);
        
        if (tentativas >= MAX_TENTATIVAS) {
            printf(RED "Acesso negado.\n" RESET);
            return;
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
        printf("8. Ordenar Eventos\n");
        printf("0. Voltar\n");
        printf(CYAN "Opcao: " RESET);
        
        if(scanf("%d", &op) != 1) {
            limparBuffer();
            printf(RED "\nEntrada invalida!\n" RESET);
            continue;
        }
        limparBuffer();

        switch (op) {
            case 1:
                cadastrarEvento();
                break;
            case 2:
                listarEventos();
                break;
            case 3:
                listarTodasInscricoes();
                break;
            case 4:
                listarFilaEspera();
                break;
            case 5:
                adminRemoverInscricaoEspecifica();
                break;
            case 6: {
                char nome_busca[MAX];
                printf("\nDigite o nome do evento: ");
                obterStringSegura(nome_busca, MAX, "");
                Evento *e = buscaIndexada(nome_busca);
                
                if (e) {
                    printf(GREEN "\nEvento encontrado:\n" RESET);
                    printf("Nome: %s\n", e->nome);
                    printf("Categoria: %s\n", e->categoria);
                    printf("Vagas: %d/%d\n", e->max_vagas - e->vagas, e->max_vagas);
                } else {
                    printf(RED "\nEvento nao encontrado!\n" RESET);
                }
                break;
            }
            case 7:
                listarPilhaEventos();
                break;
            case 8:
                mergeSort(&eventos);
                printf(GREEN "Eventos ordenados com sucesso!\n" RESET);
                break;
            case 0:
                return;
            default:
                printf(RED "Opcao invalida!\n" RESET);
        }
    }
}

// Função principal
int main() {
    // Inicializa sistema
    inicializarHashParticipantes();
    carregarTudoCSV();

    int op;
    while (1) {
        printf("\n" CYAN "=== SISTEMA DE EVENTOS ===\n" RESET);
        printf("1. Sou Usuario\n");
        printf("2. Sou Administrador\n");
        printf("0. Sair\n");
        printf(CYAN "Opcao: " RESET);
        
        if(scanf("%d", &op) != 1) {
            limparBuffer();
            printf(RED "\nEntrada invalida!\n" RESET);
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
                salvarTudoCSV();
                liberarMemoria();
                printf(GREEN "Sistema encerrado!\n" RESET);
                return 0;
            default:
                printf(RED "Opcao invalida!\n" RESET);
        }
    }
}