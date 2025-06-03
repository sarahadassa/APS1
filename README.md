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
