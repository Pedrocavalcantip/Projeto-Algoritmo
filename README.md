# Simulador de Fila de Atendimento em C++

Este projeto é um simulador de eventos discretos para uma fila de atendimento, implementado em C++. Ele processa eventos de chegada de clientes e chamadas para atendimento, calculando métricas de desempenho como tempo médio de espera e utilização.

## Funcionalidades

- **Simulação baseada em eventos:** Processa eventos de `CHEGADA`, `CHAMAR` e `FIM`.
- **Fila dinâmica:** A fila de clientes cresce dinamicamente conforme a necessidade, realocando memória.
- **Métricas de Desempenho:** Calcula e exibe métricas detalhadas, incluindo:
  - Total de clientes atendidos.
  - Tempo médio de espera.
  - Clientes restantes na fila.
  - Contadores de operações (inserções, remoções, comparações, realocações).
- **Saída em Arquivo:** Gera um log de eventos detalhado e um resumo das métricas em um arquivo de texto.
- **Métricas em Binário:** Salva as métricas finais em um arquivo binário para processamento posterior.
- **Modo Interativo:** Pode ser executado interativamente, lendo comandos do terminal.

## Como Compilar

É necessário um compilador C++ (como g++ ou Clang). No Windows, você pode usar o MinGW/MSYS2 ou o Build Tools do Visual Studio.

### Usando g++

Navegue até o diretório do projeto e execute o seguinte comando:

```bash
g++ -O2 -std=c++17 -o simulacao simulacao_fila.cpp
```

Isso irá gerar um executável chamado `simulacao.exe` (no Windows) ou `simulacao` (no Linux/macOS).

## Como Executar

O programa pode ser executado de duas formas: modo de arquivo ou modo interativo.

### 1. Modo de Arquivo

Este é o modo principal, onde os eventos são lidos de um arquivo de entrada e a saída é gravada em arquivos de texto e binário.

**Comando:**

```powershell
.\simulacao.exe <arquivo_de_entrada> <arquivo_de_saida> [arquivo_de_metricas_bin]
```

**Exemplo de Compilação e Execução:**

Primeiro, compile o programa:
```powershell
g++ -O2 -std=c++17 -o simulacao.exe simulacao_fila.cpp
```

Depois, execute com um arquivo de exemplo:
```powershell
.\simulacao.exe exemplo_completo.in saida_completa.txt
```

- `exemplo_completo.in`: Arquivo com a sequência de eventos.
- `saida_completa.txt`: Arquivo onde o log da simulação e o resumo serão salvos.
- `metricas.bin`: (Opcional) Arquivo onde as métricas serão salvas em formato binário.

### 2. Modo Interativo

Neste modo, o programa lê os eventos diretamente do terminal e imprime a saída no mesmo.

**Comando:**

```powershell
.\simulacao.exe -
```

Você pode então digitar os eventos um por um. Para encerrar, digite `FIM` e pressione Enter, seguido por `Ctrl+Z` (no Windows) ou `Ctrl+D` (no Linux/macOS) para sinalizar o fim da entrada.

## Formato do Arquivo de Entrada

O arquivo de entrada deve conter uma sequência de eventos, um por linha. Linhas vazias ou iniciadas com `#` são ignoradas.

### `CHEGADA`
Registra a chegada de um cliente na fila.

- **Formato:** `CHEGADA <id> <momento> <grupo> <prioridade> <tolerancia>`
- **Exemplo:** `CHEGADA 1 10 4 2 15`

### `CHAMAR`
Tenta atender o próximo cliente da fila no momento especificado.

- **Formato:** `CHAMAR <momento>`
- **Exemplo:** `CHAMAR 20`

### `FIM`
Encerra a simulação.

- **Formato:** `FIM`

## Formato do Arquivo de Saída

O arquivo de saída contém um log de todos os eventos processados, seguido por um resumo das métricas da simulação.

### Log de Eventos
Cada linha representa um evento processado, no formato:
`momento, TIPO, detalhes...`

### Resumo das Métricas
Após a linha `---`, as seguintes métricas são apresentadas:
- `ATENDIDOS`: Número total de clientes atendidos.
- `AVG_ESPERA`: Tempo médio de espera dos clientes atendidos.
- `FILA_RESTANTE`: Número de clientes que permaneceram na fila ao final.
- `INSERCOES`, `REMOCOES`, `COMPARACOES`: Contadores de operações da fila.
- `REALOCACOES`, `COPIAS`: Contadores de gerenciamento de memória da fila.
