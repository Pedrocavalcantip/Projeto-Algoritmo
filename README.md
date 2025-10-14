# Simulador de Fila de Atendimento em C++

## Descrição

Este projeto implementa um simulador de eventos discretos para gerenciamento de filas de atendimento, desenvolvido em C++. O sistema modela o comportamento de um estabelecimento com múltiplas mesas de atendimento, processando eventos de chegada de clientes, chamadas para atendimento e abandonos por excesso de tempo de espera.

## Objetivos

O simulador foi desenvolvido com os seguintes objetivos:
- Modelar o comportamento de filas de atendimento em tempo real.
- Analisar métricas de desempenho e eficiência operacional.
- Implementar estruturas de dados eficientes (fila circular e min-heap).
- Demonstrar o gerenciamento dinâmico de memória em C++.
- Simular cenários realistas de abandono de clientes por intolerância à espera.

## Funcionalidades

### Características Principais

- **Simulação baseada em eventos discretos:** Processa eventos temporais do tipo `CHEGADA`, `CHAMAR` e `FIM`.
- **Múltiplas mesas de atendimento:** Sistema configurável com 3 mesas operando simultaneamente (definido pela constante `NUM_MESAS`).
- **Fila dinâmica circular:** Implementação de fila circular com realocação automática de memória quando necessário.
- **Min-heap para gerenciamento de mesas:** Utiliza heap mínimo para alocação eficiente da próxima mesa disponível.
- **Sistema de abandono:** Clientes abandonam a fila quando o tempo de espera excede sua tolerância individual.
- **Estrutura de dados Cliente:** Cada cliente possui:
  - ID único
  - Momento de chegada
  - Tamanho do grupo
  - Nível de prioridade
  - Tolerância de espera (em unidades de tempo)

### Métricas Calculadas

O simulador calcula e exibe as seguintes métricas de desempenho:
- Total de clientes atendidos com sucesso
- Tempo médio de espera dos clientes atendidos
- Número de clientes restantes na fila ao final da simulação
- Contadores de operações na fila (inserções, remoções, comparações)
- Estatísticas de gerenciamento de memória (realocações e cópias)

### Formatos de Saída

- **Arquivo de texto:** Log detalhado de todos os eventos processados e resumo estatístico.
- **Arquivo binário:** Métricas serializadas para processamento posterior ou integração com outras ferramentas.
- **Modo interativo:** Saída em tempo real no console para testes e demonstrações.

## Estrutura do Projeto

```
Projeto-Algoritmo/
├── simulacao_fila.cpp         # Código-fonte principal
├── exemplo_completo.in        # Arquivo de entrada com cenários de teste
├── input.in                   # Arquivo de entrada simples
├── README.md                  # Documentação do projeto
└── saida_completa.txt         # Exemplo de saída gerada
```

## Requisitos

- **Compilador C++:** g++ (MinGW/MSYS2) ou MSVC com suporte a C++17 ou superior
- **Sistema Operacional:** Windows, Linux ou macOS
- **Padrão C++:** C++17 ou superior

## Como Compilar

### Compilação no Windows (PowerShell)

Navegue até o diretório do projeto e execute:

```powershell
g++ -O2 -std=c++17 -o simulacao.exe simulacao_fila.cpp
```

### Compilação no Linux/macOS

```bash
g++ -O2 -std=c++17 -o simulacao simulacao_fila.cpp
```

O compilador irá gerar um executável otimizado com o nome `simulacao.exe` (Windows) ou `simulacao` (Linux/macOS).

## Como Executar

O programa oferece dois modos de execução: modo de arquivo (recomendado) e modo interativo.

### 1. Modo de Arquivo (Recomendado)

Este é o modo principal de operação, onde os eventos são lidos de um arquivo de entrada e a saída é gravada em arquivos de texto e/ou binário.

**Sintaxe:**

```powershell
.\simulacao.exe <arquivo_de_entrada> <arquivo_de_saida> [arquivo_de_metricas_bin]
```

**Exemplo de Uso com Arquivo de Teste Completo:**

O projeto inclui um arquivo `exemplo_completo.in` que testa todos os recursos do simulador, incluindo múltiplas mesas, sistema de abandono e realocação de memória.

Primeiro, compile o programa:
```powershell
g++ -O2 -std=c++17 -o simulacao.exe simulacao_fila.cpp
```

Depois, execute com o arquivo de exemplo:
```powershell
.\simulacao.exe exemplo_completo.in saida_completa.txt
```

**Parâmetros:**
- `exemplo_completo.in`: Arquivo de entrada contendo a sequência de eventos a serem simulados
- `saida_completa.txt`: Arquivo de saída onde o log detalhado e o resumo estatístico serão gravados
- `metricas.bin` (opcional): Arquivo binário para armazenar as métricas finais

### 2. Modo Interativo

Neste modo, o programa lê os eventos diretamente da entrada padrão (terminal) e exibe a saída no console.

**Comando:**

```powershell
.\simulacao.exe -
```

**Uso:**
1. Execute o comando acima
2. Digite os eventos linha por linha
3. Pressione `Ctrl+Z` (Windows) ou `Ctrl+D` (Linux/macOS) seguido de Enter para finalizar a entrada
4. A saída será exibida imediatamente no console

**Nota:** O modo interativo é útil para testes rápidos e demonstrações, mas o modo de arquivo é recomendado para análises mais complexas.

## Formato do Arquivo de Entrada

O arquivo de entrada deve conter uma sequência de eventos, um por linha. Linhas vazias ou iniciadas com `#` são tratadas como comentários e ignoradas pelo parser.

### Tipos de Eventos

#### 1. `CHEGADA` - Registro de Chegada de Cliente

Registra a entrada de um novo cliente na fila de espera.

- **Sintaxe:** `CHEGADA <id> <momento> <tamanho_grupo> <prioridade> <tolerancia>`
- **Parâmetros:**
  - `id`: Identificador único do cliente (inteiro)
  - `momento`: Momento temporal da chegada (inteiro)
  - `tamanho_grupo`: Número de pessoas no grupo (inteiro)
  - `prioridade`: Nível de prioridade do atendimento (inteiro)
  - `tolerancia`: Tempo máximo de espera tolerado antes de abandonar (inteiro)
- **Exemplo:** `CHEGADA 101 10 4 2 15`
  - Cliente ID 101 chega no momento 10, grupo de 4 pessoas, prioridade 2, tolera esperar até 15 unidades de tempo

#### 2. `CHAMAR` - Tentativa de Atendimento

Tenta atender o próximo cliente da fila no momento especificado.

- **Sintaxe:** `CHAMAR <momento>`
- **Parâmetros:**
  - `momento`: Momento temporal da tentativa de chamada (inteiro)
- **Exemplo:** `CHAMAR 20`
  - No momento 20, tenta atender o próximo cliente disponível

**Comportamento do Sistema:**
- Se houver mesa disponível e cliente na fila, inicia atendimento
- Se todas as mesas estiverem ocupadas, informa o momento de liberação da próxima mesa
- Se o cliente excedeu sua tolerância, registra abandono e tenta o próximo da fila
- Se a fila estiver vazia, informa "fila_vazia"

#### 3. `FIM` - Encerramento da Simulação

Finaliza a simulação e gera o relatório de métricas.

- **Sintaxe:** `FIM`
- **Exemplo:** `FIM`

### Exemplo de Arquivo de Entrada Completo

O projeto inclui o arquivo `exemplo_completo.in`, que demonstra todos os cenários de teste:

```
# Múltiplas chegadas
CHEGADA 101 0 2 1 50
CHEGADA 102 2 4 1 50
CHEGADA 103 4 3 2 50

# Atendimento simultâneo (3 mesas)
CHAMAR 10
CHAMAR 10
CHAMAR 10

# Cliente com baixa tolerância (teste de abandono)
CHEGADA 201 20 2 1 5
CHAMAR 35

# Finalização
FIM
```

Este arquivo testa:
- Atendimento simultâneo em múltiplas mesas
- Sistema de abandono por excesso de tempo
- Realocação dinâmica de memória da fila
- Comportamento com fila vazia

## Formato do Arquivo de Saída

O arquivo de saída é dividido em duas seções principais: log de eventos e resumo estatístico.

### Seção 1: Log de Eventos

Cada linha representa um evento processado durante a simulação, seguindo o formato:

```
<momento>, <TIPO_EVENTO>, <detalhes>
```

**Tipos de eventos registrados:**

1. **CHEGADA**: Registro de chegada de cliente
   ```
   10, CHEGADA, id=101 grupo=2 prioridade=1 tolerancia=50
   ```

2. **CHAMADA com atendimento**: Cliente é atendido
   ```
   10, CHAMADA, id=101 mesa=0 inicia_atendimento; espera=10; termina_em=40
   ```

3. **CHAMADA com mesa ocupada**: Todas as mesas estão ocupadas
   ```
   15, CHAMADA, nenhuma mesa livre (prox_livre=40)
   ```

4. **CHAMADA com abandono**: Cliente excedeu tolerância
   ```
   35, CHAMADA, ABANDONO id=201 esperou=15 > tolerancia=5
   ```

5. **CHAMADA com fila vazia**: Não há clientes aguardando
   ```
   160, CHAMADA, fila_vazia
   ```

6. **FIM**: Encerramento da simulação
   ```
   160, FIM
   ```

### Seção 2: Resumo Estatístico

Após o separador `---`, são apresentadas as métricas finais da simulação:

```
---
ATENDIDOS=8
AVG_ESPERA=25.50
FILA_RESTANTE=0
INSERCOES=13 REMOCOES=14 COMPARACOES=12
REALOCACOES=1 COPIAS=8
```

**Descrição das métricas:**

- `ATENDIDOS`: Número total de clientes que foram atendidos com sucesso
- `AVG_ESPERA`: Tempo médio de espera dos clientes atendidos (em unidades de tempo)
- `FILA_RESTANTE`: Número de clientes que permaneceram na fila ao final da simulação
- `INSERCOES`: Total de operações de inserção na fila
- `REMOCOES`: Total de operações de remoção da fila
- `COMPARACOES`: Número de comparações realizadas
- `REALOCACOES`: Quantidade de vezes que a fila precisou realocar memória
- `COPIAS`: Número total de cópias de elementos durante realocações

### Arquivo Binário de Métricas (Opcional)

Quando especificado, o programa gera um arquivo binário contendo a estrutura `Metricas` serializada, útil para:
- Processamento posterior por outras ferramentas
- Análise estatística automatizada
- Integração com sistemas de visualização de dados

## Constantes de Configuração

O simulador possui constantes configuráveis no código-fonte:

```cpp
#define TEMPO_ATENDIMENTO 30  // Duração fixa de cada atendimento
#define NUM_MESAS 3           // Número de mesas disponíveis
```

Para modificar o comportamento do sistema, altere essas constantes e recompile o programa.

## Estruturas de Dados Utilizadas

### Fila Circular Dinâmica
- Implementação eficiente com ponteiros de início e fim
- Realocação automática com fator de crescimento 2x
- Complexidade O(1) para inserção e remoção (amortizado)

### Min-Heap para Gerenciamento de Mesas
- Heap binário mínimo para alocação eficiente de mesas
- Complexidade O(log n) para inserção e remoção
- Garante sempre a alocação da mesa que ficará livre primeiro

### Estrutura Cliente
```cpp
struct Cliente {
    int id_cliente;
    int momento_chegada;
    int tamanho_grupo;
    int prioridade;
    int tolerancia_espera;
};
```

## Análise de Complexidade

- **Inserção na fila**: O(1) amortizado
- **Remoção da fila**: O(1)
- **Gerenciamento de mesas (heap)**: O(log k), onde k = NUM_MESAS
- **Processamento de evento CHEGADA**: O(1) amortizado
- **Processamento de evento CHAMAR**: O(log k + m), onde m é o número de abandonos consecutivos

## Autores

Projeto desenvolvido para a disciplina de Algoritmos.

## Licença

Este projeto é de código aberto e está disponível para fins educacionais.
