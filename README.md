# Sistema de Gerenciamento de Restaurante
## Entrega 2 - Estruturas de Dados Avançadas

### 1. Descrição do Projeto
Este projeto consiste em um sistema de simulação para gerenciamento de filas em um restaurante, implementado em C++. O sistema utiliza estruturas de dados avançadas para gerenciar prioridades de atendimento, alocação de mesas e análise de métricas de desempenho, garantindo eficiência e escalabilidade.

### 2. Funcionalidades Implementadas
*   **Fila de Prioridade (Max-Heap):** Gerenciamento de clientes onde aqueles com maior prioridade são atendidos primeiro.
*   **Gerenciamento de Mesas (Min-Heap):** Alocação eficiente de mesas, priorizando as que serão liberadas mais cedo.
*   **Ordenação (QuickSort):** Organização do histórico de atendimentos por identificador do cliente.
*   **Ordenação Estável (MergeSort):** Organização do histórico por tempo de espera.
*   **Busca Eficiente (Busca Binária):** Localização rápida de registros de clientes específicos no histórico.
*   **Relatórios Estatísticos:** Geração de métricas detalhadas sobre o desempenho do sistema e operações realizadas.

### 3. Requisitos de Sistema
*   Compilador C++ compatível com o padrão C++11 ou superior (ex: g++, MinGW).
*   Sistema Operacional: Windows, Linux ou macOS.
*   PowerShell (recomendado para execução dos scripts de teste automatizados no Windows).

### 4. Compilação
Para compilar o projeto, utilize os comandos abaixo conforme seu ambiente de desenvolvimento.

**Windows (PowerShell):**
`powershell
cd "etapa 2"
g++ -std=c++11 -Wall -Wextra -O2 sistema_restaurante.cpp -o restaurante.exe
`

**Linux/macOS:**
`bash
cd "etapa 2"
g++ -std=c++11 -Wall -Wextra -O2 sistema_restaurante.cpp -o restaurante
`

### 5. Execução
O sistema pode ser executado em dois modos distintos.

**Modo Arquivo (Recomendado):**
Processa um arquivo de entrada contendo uma lista de comandos e gera um arquivo de saída com os resultados.
`powershell
.\restaurante.exe exemplo.in resultado.txt
`

**Modo Interativo:**
Permite a entrada manual de comandos via terminal.
`powershell
.\restaurante.exe
`

### 6. Automação de Testes
O projeto inclui um script em PowerShell (gerar_testes.ps1) para geração automática de cenários de teste variados.

**Comandos Disponíveis:**

*   **Teste Básico (5 clientes):**
    `powershell
    .\gerar_testes.ps1 basico
    `
*   **Teste Médio (50 clientes):**
    `powershell
    .\gerar_testes.ps1 medio
    `
*   **Teste de Carga (1000 clientes - Benchmark):**
    `powershell
    .\gerar_testes.ps1 grande
    `
*   **Teste de Prioridades:**
    `powershell
    .\gerar_testes.ps1 prioridades
    `
*   **Teste de Abandonos (Timeout):**
    `powershell
    .\gerar_testes.ps1 abandonos
    `
*   **Gerar Todos os Testes:**
    `powershell
    .\gerar_testes.ps1 todos
    `

**Execução dos Testes Gerados:**
`powershell
.\restaurante.exe teste_basico.in saida_basico.txt
Get-Content saida_basico.txt
`

### 7. Formato de Entrada
O sistema aceita os seguintes comandos no arquivo de entrada ou via terminal:

*   CHEGADA <id> <momento> <tamanho_grupo> <prioridade> <tolerancia>: Registra a chegada de um novo cliente.
*   CHAMAR <momento>: Realiza o atendimento do próximo cliente da fila de prioridade.
*   RELATORIO <tipo>: Gera relatórios ordenados. Tipos aceitos: ID (QuickSort) ou ESPERA (MergeSort).
*   BUSCAR <id>: Localiza um cliente específico no histórico de atendimentos usando busca binária.
*   FIM: Encerra a simulação e exibe as métricas finais.

### 8. Análise de Complexidade
As seguintes complexidades assintóticas são esperadas para as operações principais implementadas:

*   **Inserção de Cliente (Max-Heap):** O(log n)
*   **Atendimento (Max-Heap + Min-Heap):** O(log n)
*   **Ordenação por ID (QuickSort):** O(n log n)
*   **Ordenação por Espera (MergeSort):** O(n log n)
*   **Busca de Cliente (Busca Binária):** O(log n)

### 9. Justificativa de Estruturas
*   **Grafos:** Não foram utilizados nesta implementação pois o domínio do problema (fila de restaurante) é linear e não apresenta relações de conectividade complexa ou necessidade de cálculos de caminho mínimo que justificariam o uso de grafos.
*   **Árvores Balanceadas:** A estrutura de Heap foi selecionada por oferecer desempenho superior e implementação mais direta para as operações de fila de prioridade requeridas pelo sistema.
