# Sistema de Cadastro de Eventos em C

Este é um sistema de gerenciamento de eventos desenvolvido em linguagem C, com foco na prática e aplicação de diversas estruturas de dados e conceitos de modularização e Clean Code.

## 🚀 Funcionalidades

O sistema oferece as seguintes funcionalidades principais:

* **Cadastro de Eventos:** Adição de novos eventos com nome, categoria e número de vagas.
* **Gestão de Participantes:** Registro de participantes e sua associação a eventos.
* **Inscrição em Eventos:** Usuários podem se inscrever em eventos disponíveis.
* **Fila de Espera:** Gerenciamento automático de participantes em fila de espera caso um evento esteja lotado.
* **Remoção de Inscrições:** Usuários podem remover suas próprias inscrições; administradores podem remover inscrições específicas.
* **Listagem Completa:** Visualização de todos os eventos, inscrições e fila de espera.
* **Histórico de Eventos:** Uma pilha (stack) mantém um registro das ações de inscrição em eventos.
* **Busca Otimizada:** Eventos podem ser buscados rapidamente por nome utilizando um índice (array de ponteiros) e busca binária.
* **Persistência de Dados:** Todos os dados são salvos e carregados de arquivos CSV, garantindo que as informações não sejam perdidas ao encerrar o programa.
* **Autenticação de Administrador:** Um menu de administrador protegido por senha oferece controle total sobre o sistema.

## 📁 Estrutura do Projeto

O projeto segue uma estrutura modular para facilitar a leitura, manutenção e escalabilidade, dividindo as funcionalidades em diretórios e pares de arquivos `.h` e `.c`.


├── src/
│   ├── comum.h                  # Definições globais, structs e includes básicos
│   ├── Evento/
│   │   ├── eventos.h            # Protótipos das funções de gestão de eventos
│   │   └── eventos.c            # Implementação das funções de gestão de eventos
│   ├── FilaPilha/
│   │   ├── filaPilha.h          # Protótipos das funções de Fila de Espera e Pilha de Eventos
│   │   └── filaPilha.c          # Implementação das funções de Fila de Espera e Pilha de Eventos
│   ├── Inscricoes/
│   │   ├── inscricoes.h         # Protótipos das funções de gestão de inscrições
│   │   └── inscricoes.c         # Implementação das funções de gestão de inscrições
│   ├── Participantes/
│   │   ├── participantes.h      # Protótipos das funções de gestão de participantes (hash table)
│   │   └── participantes.c      # Implementação das funções de gestão de participantes
│   ├── Persistencia/
│   │   ├── persist.h            # Protótipos das funções de salvamento/carregamento de dados (CSV)
│   │   └── persist.c            # Implementação das funções de salvamento/carregamento de dados
│   ├── Utils/
│   │   ├── utils.h              # Protótipos de funções utilitárias gerais
│   │   └── utils.c              # Implementação das funções utilitárias
│   └── main.c                   # Lógica principal do programa e menus de interação
└── README.md


## 🛠️ Estruturas de Dados Utilizadas

* **Listas Encadeadas:** Para `Eventos`, `Inscrições` e internamente para `Participantes` em cada evento (lista circular).
* **Tabela Hash:** Para armazenar e buscar `Participantes` de forma eficiente por e-mail.
* **Fila (Queue):** Para gerenciar a `Fila de Espera` por vagas em eventos lotados.
* **Pilha (Stack):** Para manter um `Histórico` de eventos.
* **Array de Ponteiros (Índice):** Para permitir busca binária rápida de eventos por nome.

## ⚙️ Como Compilar

Para compilar este projeto, você precisará de um compilador C (como o GCC).

1.  **Navegue até a pasta raiz do projeto** (onde está o `src` e este `README.md`).

2.  **Execute o seguinte comando no terminal:**

gcc src/main.c src/Utils/utils.c src/Evento/eventos.c src/Participantes/participantes.c src/Inscricoes/inscricoes.c src/FilaPilha/filaPilha.c src/Persistencia/persist.c -I"src" -I"src/Utils" -I"src/Evento" -I"src/Participantes" -I"src/Inscricoes" -I"src/FilaPilha" -I"src/Persistencia" -o sistema_eventos

## ▶️ Como Executar

Após a compilação bem-sucedida:

* **No Linux/macOS:**

    ./sistema_eventos

* **No Windows:**

    .\sistema_eventos.exe


## 🔑 Acesso Administrativo

A senha padrão de administrador é: `admin123`

## ⚠️ Observações

* Ocultar a digitação da senha no `menuAdmin` (função `_getch()`) é nativa do Windows (`conio.h`). Em sistemas Unix-like (Linux/macOS), a senha será visível a menos que sejam usadas bibliotecas como `termios.h` para controle de terminal.
* Este projeto é uma prática de Estruturas de Dados. Algumas implementações (como a estratégia de colisão da Hash Table) são simplificadas para fins didáticos.


