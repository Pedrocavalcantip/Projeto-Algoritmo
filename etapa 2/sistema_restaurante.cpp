#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <vector>

using namespace std;

#define TEMPO_ATENDIMENTO 30
#define NUM_MESAS 3

enum TipoEvento { EVT_CHEGADA, EVT_CHAMAR, EVT_FIM, EVT_RELATORIO, EVT_BUSCAR, EVT_INVALIDO };

struct Cliente {
    int id_cliente;
    int momento_chegada;
    int tamanho_grupo;
    int prioridade;
    int tolerancia_espera;
};

struct AtendimentoRegistro {
    int id_cliente;
    int momento_inicio;
    int tempo_espera;
    int mesa_id;
};

// Heap de prioridade para clientes (max-heap por prioridade)
struct HeapPrioridade {
    Cliente *dados;
    int n, cap;
    long long comparacoes;
};

struct MinHeapMesas {
    int *livre_em;
    int *id;
    int n, cap;
};

struct Metricas {
    int total_atendidos;
    long long soma_tempos_espera;
    int tamanho_fila_restante;
    long long cont_insercoes;
    long long cont_remocoes;
    long long cont_comparacoes;
    long long cont_realocacoes;
    long long cont_copias;
    long long cont_ordenacoes;
    long long cont_buscas;
};

struct Simulacao {
    int momento_atual;
    Metricas metricas;
    MinHeapMesas mesas;
    vector<AtendimentoRegistro> historico;
};

// ========== HEAP DE MESAS (Min-Heap) ==========
void heap_swap_mesas(MinHeapMesas *h, int i, int j){
    int te = h->livre_em[i], ti = h->id[i];
    h->livre_em[i] = h->livre_em[j]; h->id[i] = h->id[j];
    h->livre_em[j] = te; h->id[j] = ti;
}

void heap_init_mesas(MinHeapMesas *h, int cap){
    h->cap = (cap < 1 ? 1 : cap);
    h->n = 0;
    h->livre_em = new int[h->cap];
    h->id = new int[h->cap];
}

void heap_free_mesas(MinHeapMesas *h){
    delete[] h->livre_em; h->livre_em = nullptr;
    delete[] h->id; h->id = nullptr;
    h->n = h->cap = 0;
}

void heap_push_mesas(MinHeapMesas *h, int livre, int mesa_id){
    if(h->n == h->cap){
        int nova = h->cap * 2;
        int *ne = new int[nova];
        int *ni = new int[nova];
        for(int i=0;i<h->n;++i){ ne[i]=h->livre_em[i]; ni[i]=h->id[i]; }
        delete[] h->livre_em; delete[] h->id;
        h->livre_em = ne; h->id = ni; h->cap = nova;
    }
    int i = h->n++;
    h->livre_em[i] = livre; h->id[i] = mesa_id;
    while(i > 0){
        int p = (i - 1) / 2;
        if(h->livre_em[p] <= h->livre_em[i]) break;
        heap_swap_mesas(h, i, p); i = p;
    }
}

bool heap_pop_mesas(MinHeapMesas *h, int *livre, int *mesa_id){
    if(h->n == 0) return false;
    *livre = h->livre_em[0]; *mesa_id = h->id[0];
    h->n--;
    if(h->n > 0){
        h->livre_em[0] = h->livre_em[h->n];
        h->id[0] = h->id[h->n];
        int i = 0;
        while(true){
            int l = 2*i+1, r = 2*i+2, m = i;
            if(l < h->n && h->livre_em[l] < h->livre_em[m]) m = l;
            if(r < h->n && h->livre_em[r] < h->livre_em[m]) m = r;
            if(m == i) break;
            heap_swap_mesas(h, i, m); i = m;
        }
    }
    return true;
}

bool heap_peek_mesas(const MinHeapMesas *h, int *livre, int *mesa_id){
    if(h->n == 0) return false;
    *livre = h->livre_em[0]; *mesa_id = h->id[0];
    return true;
}

// ========== HEAP DE PRIORIDADE PARA CLIENTES (Max-Heap) ==========
void heap_init_prioridade(HeapPrioridade *h, int cap){
    h->cap = (cap < 1 ? 4 : cap);
    h->n = 0;
    h->dados = new Cliente[h->cap];
    h->comparacoes = 0;
}

void heap_free_prioridade(HeapPrioridade *h){
    delete[] h->dados;
    h->dados = nullptr;
    h->n = h->cap = 0;
    h->comparacoes = 0;
}

void heap_swap_prioridade(HeapPrioridade *h, int i, int j){
    Cliente tmp = h->dados[i];
    h->dados[i] = h->dados[j];
    h->dados[j] = tmp;
}

bool comparar_cliente(const Cliente &a, const Cliente &b, long long *comp){
    (*comp)++;
    if(a.prioridade != b.prioridade)
        return a.prioridade > b.prioridade;
    return a.momento_chegada < b.momento_chegada;
}

void heap_push_prioridade(HeapPrioridade *h, const Cliente &c, Metricas *m){
    if(h->n == h->cap){
        int nova = h->cap * 2;
        Cliente *nd = new Cliente[nova];
        for(int i=0;i<h->n;++i) nd[i] = h->dados[i];
        delete[] h->dados;
        h->dados = nd; h->cap = nova;
        if(m) m->cont_realocacoes++;
    }
    int i = h->n++;
    h->dados[i] = c;
    while(i > 0){
        int p = (i - 1) / 2;
        if(!comparar_cliente(h->dados[i], h->dados[p], &h->comparacoes)) break;
        heap_swap_prioridade(h, i, p);
        i = p;
    }
    if(m) m->cont_insercoes++;
}

bool heap_pop_prioridade(HeapPrioridade *h, Cliente *destino, Metricas *m){
    if(h->n == 0) return false;
    *destino = h->dados[0];
    h->n--;
    if(h->n > 0){
        h->dados[0] = h->dados[h->n];
        int i = 0;
        while(true){
            int l = 2*i+1, r = 2*i+2, m_idx = i;
            if(l < h->n && comparar_cliente(h->dados[l], h->dados[m_idx], &h->comparacoes))
                m_idx = l;
            if(r < h->n && comparar_cliente(h->dados[r], h->dados[m_idx], &h->comparacoes))
                m_idx = r;
            if(m_idx == i) break;
            heap_swap_prioridade(h, i, m_idx);
            i = m_idx;
        }
    }
    if(m) m->cont_remocoes++;
    return true;
}

bool heap_vazia_prioridade(const HeapPrioridade *h){
    return h->n == 0;
}

// ========== ALGORITMOS DE ORDENAÇÃO ==========
// QuickSort para ordenar histórico por ID
int particionar(AtendimentoRegistro arr[], int low, int high, Metricas *m){
    int pivot = arr[high].id_cliente;
    int i = low - 1;
    for(int j = low; j < high; j++){
        m->cont_comparacoes++;
        if(arr[j].id_cliente < pivot){
            i++;
            swap(arr[i], arr[j]);
            m->cont_copias += 3;
        }
    }
    swap(arr[i + 1], arr[high]);
    m->cont_copias += 3;
    return i + 1;
}

void quicksort(AtendimentoRegistro arr[], int low, int high, Metricas *m){
    if(low < high){
        int pi = particionar(arr, low, high, m);
        quicksort(arr, low, pi - 1, m);
        quicksort(arr, pi + 1, high, m);
    }
}

// MergeSort para ordenar por tempo de espera
void merge(AtendimentoRegistro arr[], int l, int m, int r, Metricas *met){
    int n1 = m - l + 1;
    int n2 = r - m;
    AtendimentoRegistro *L = new AtendimentoRegistro[n1];
    AtendimentoRegistro *R = new AtendimentoRegistro[n2];
    
    for(int i = 0; i < n1; i++){
        L[i] = arr[l + i];
        met->cont_copias++;
    }
    for(int j = 0; j < n2; j++){
        R[j] = arr[m + 1 + j];
        met->cont_copias++;
    }
    
    int i = 0, j = 0, k = l;
    while(i < n1 && j < n2){
        met->cont_comparacoes++;
        if(L[i].tempo_espera <= R[j].tempo_espera){
            arr[k] = L[i++];
        } else {
            arr[k] = R[j++];
        }
        met->cont_copias++;
        k++;
    }
    
    while(i < n1){
        arr[k++] = L[i++];
        met->cont_copias++;
    }
    while(j < n2){
        arr[k++] = R[j++];
        met->cont_copias++;
    }
    
    delete[] L;
    delete[] R;
}

void mergesort(AtendimentoRegistro arr[], int l, int r, Metricas *m){
    if(l < r){
        int mid = l + (r - l) / 2;
        mergesort(arr, l, mid, m);
        mergesort(arr, mid + 1, r, m);
        merge(arr, l, mid, r, m);
    }
}

// ========== BUSCA BINÁRIA ==========
int busca_binaria(AtendimentoRegistro arr[], int n, int id_cliente, Metricas *m){
    int left = 0, right = n - 1;
    while(left <= right){
        m->cont_buscas++;
        int mid = left + (right - left) / 2;
        m->cont_comparacoes++;
        if(arr[mid].id_cliente == id_cliente)
            return mid;
        m->cont_comparacoes++;
        if(arr[mid].id_cliente < id_cliente)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return -1;
}

// ========== SIMULAÇÃO ==========
TipoEvento interpretar_tipo_evento(const string &tok){
    if(tok == "CHEGADA") return EVT_CHEGADA;
    if(tok == "CHAMAR") return EVT_CHAMAR;
    if(tok == "FIM") return EVT_FIM;
    if(tok == "RELATORIO") return EVT_RELATORIO;
    if(tok == "BUSCAR") return EVT_BUSCAR;
    return EVT_INVALIDO;
}

void simulacao_iniciar(Simulacao *s){
    s->momento_atual = 0;
    memset(&s->metricas, 0, sizeof(Metricas));
    heap_init_mesas(&s->mesas, NUM_MESAS);
    for(int k=0; k<NUM_MESAS; ++k){
        heap_push_mesas(&s->mesas, 0, k);
    }
    s->historico.clear();
}

void processar_chegada(Simulacao *s, HeapPrioridade *fila, const Cliente *c, ostream *saida){
    if(c->momento_chegada > s->momento_atual)
        s->momento_atual = c->momento_chegada;
    *saida << s->momento_atual << ", CHEGADA, id=" << c->id_cliente
           << " grupo=" << c->tamanho_grupo
           << " prioridade=" << c->prioridade
           << " tolerancia=" << c->tolerancia_espera << "\n";
    heap_push_prioridade(fila, *c, &s->metricas);
}

void processar_chamada(Simulacao *s, HeapPrioridade *fila, int momento_chamada, ostream *saida){
    if(momento_chamada > s->momento_atual)
        s->momento_atual = momento_chamada;
    
    while(!heap_vazia_prioridade(fila)){
        Cliente topo;
        heap_pop_prioridade(fila, &topo, &s->metricas);
        int esperou = s->momento_atual - topo.momento_chegada;
        s->metricas.cont_comparacoes++;
        
        if(esperou > topo.tolerancia_espera){
            *saida << s->momento_atual << ", CHAMADA, ABANDONO id=" << topo.id_cliente
                   << " esperou=" << esperou
                   << " > tolerancia=" << topo.tolerancia_espera << "\n";
            continue;
        } else {
            int livre_topo, mesa_id;
            if(!heap_peek_mesas(&s->mesas, &livre_topo, &mesa_id)){
                *saida << s->momento_atual << ", CHAMADA, ERRO: sem mesas\n";
                return;
            }
            if(s->momento_atual < livre_topo){
                *saida << s->momento_atual << ", CHAMADA, nenhuma mesa livre (prox="
                       << livre_topo << ")\n";
                heap_push_prioridade(fila, topo, &s->metricas);
                return;
            }
            
            heap_pop_mesas(&s->mesas, &livre_topo, &mesa_id);
            int tempo_espera = s->momento_atual - topo.momento_chegada;
            s->metricas.soma_tempos_espera += tempo_espera;
            s->metricas.total_atendidos++;
            int termina = s->momento_atual + TEMPO_ATENDIMENTO;
            heap_push_mesas(&s->mesas, termina, mesa_id);
            
            AtendimentoRegistro reg;
            reg.id_cliente = topo.id_cliente;
            reg.momento_inicio = s->momento_atual;
            reg.tempo_espera = tempo_espera;
            reg.mesa_id = mesa_id;
            s->historico.push_back(reg);
            
            *saida << s->momento_atual << ", CHAMADA, id=" << topo.id_cliente
                   << " mesa=" << mesa_id
                   << " espera=" << tempo_espera
                   << " termina=" << termina << "\n";
            return;
        }
    }
    *saida << s->momento_atual << ", CHAMADA, fila_vazia\n";
}

void gerar_relatorio(Simulacao *s, const string &tipo, ostream *saida){
    *saida << s->momento_atual << ", RELATORIO tipo=" << tipo << "\n";
    
    if(s->historico.empty()){
        *saida << "  Nenhum atendimento registrado.\n";
        return;
    }
    
    int n = s->historico.size();
    AtendimentoRegistro *copia = new AtendimentoRegistro[n];
    for(int i=0; i<n; i++) copia[i] = s->historico[i];
    
    if(tipo == "ID"){
        quicksort(copia, 0, n-1, &s->metricas);
        s->metricas.cont_ordenacoes++;
        *saida << "  Top 5 por ID:\n";
        for(int i=0; i<min(5, n); i++){
            *saida << "    ID=" << copia[i].id_cliente
                   << " mesa=" << copia[i].mesa_id
                   << " espera=" << copia[i].tempo_espera << "\n";
        }
    } else if(tipo == "ESPERA"){
        mergesort(copia, 0, n-1, &s->metricas);
        s->metricas.cont_ordenacoes++;
        *saida << "  Top 5 maiores esperas:\n";
        for(int i=max(0, n-5); i<n; i++){
            *saida << "    ID=" << copia[i].id_cliente
                   << " espera=" << copia[i].tempo_espera
                   << " mesa=" << copia[i].mesa_id << "\n";
        }
    }
    
    delete[] copia;
}

void buscar_cliente(Simulacao *s, int id_busca, ostream *saida){
    *saida << s->momento_atual << ", BUSCAR id=" << id_busca << "\n";
    
    if(s->historico.empty()){
        *saida << "  Nenhum registro encontrado.\n";
        return;
    }
    
    int n = s->historico.size();
    AtendimentoRegistro *copia = new AtendimentoRegistro[n];
    for(int i=0; i<n; i++) copia[i] = s->historico[i];
    
    quicksort(copia, 0, n-1, &s->metricas);
    int idx = busca_binaria(copia, n, id_busca, &s->metricas);
    
    if(idx != -1){
        *saida << "  ENCONTRADO: ID=" << copia[idx].id_cliente
               << " mesa=" << copia[idx].mesa_id
               << " inicio=" << copia[idx].momento_inicio
               << " espera=" << copia[idx].tempo_espera << "\n";
    } else {
        *saida << "  Cliente ID=" << id_busca << " nao encontrado.\n";
    }
    
    delete[] copia;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    istream *entrada = &cin;
    ostream *saida = &cout;
    bool interativo = (argc == 1) || (argc >= 2 && string(argv[1]) == "-");
    
    if(!interativo){
        if(argc < 3){
            cerr << "Uso: " << argv[0] << " input.in output.txt\n";
            return 1;
        }
        entrada = new ifstream(argv[1]);
        if(!*entrada){ cerr << "Erro ao abrir '" << argv[1] << "'.\n"; return 1; }
        saida = new ofstream(argv[2]);
        if(!*saida){ cerr << "Erro ao criar '" << argv[2] << "'.\n"; return 1; }
    }
    
    HeapPrioridade fila;
    heap_init_prioridade(&fila, 8);
    Simulacao sim;
    simulacao_iniciar(&sim);
    
    string linha;
    int numero_linha = 0;
    bool encerrar = false;
    
    while(!encerrar && getline(*entrada, linha)){
        numero_linha++;
        if(linha.empty() || linha[0] == '#') continue;
        
        istringstream iss(linha);
        string tok; iss >> tok;
        if(tok.empty()) continue;
        
        TipoEvento tipo = interpretar_tipo_evento(tok);
        
        if(tipo == EVT_CHEGADA){
            Cliente c;
            if(!(iss >> c.id_cliente >> c.momento_chegada >> c.tamanho_grupo 
                     >> c.prioridade >> c.tolerancia_espera)){
                cerr << "Linha " << numero_linha << " invalida.\n";
                continue;
            }
            processar_chegada(&sim, &fila, &c, saida);
        }
        else if(tipo == EVT_CHAMAR){
            int t;
            if(!(iss >> t)){ cerr << "Linha " << numero_linha << " invalida.\n"; continue; }
            processar_chamada(&sim, &fila, t, saida);
        }
        else if(tipo == EVT_RELATORIO){
            string tipo_rel;
            iss >> tipo_rel;
            gerar_relatorio(&sim, tipo_rel, saida);
        }
        else if(tipo == EVT_BUSCAR){
            int id;
            if(!(iss >> id)){ cerr << "Linha " << numero_linha << " invalida.\n"; continue; }
            buscar_cliente(&sim, id, saida);
        }
        else if(tipo == EVT_FIM){
            encerrar = true;
            *saida << sim.momento_atual << ", FIM\n";
        }
    }
    
    *saida << "---\n";
    *saida << "ATENDIDOS=" << sim.metricas.total_atendidos << "\n";
    if(sim.metricas.total_atendidos > 0){
        double media = (double)sim.metricas.soma_tempos_espera / sim.metricas.total_atendidos;
        *saida << "AVG_ESPERA=" << fixed << setprecision(2) << media << "\n";
    } else {
        *saida << "AVG_ESPERA=NA\n";
    }
    *saida << "FILA_RESTANTE=" << fila.n << "\n";
    *saida << "INSERCOES=" << sim.metricas.cont_insercoes
           << " REMOCOES=" << sim.metricas.cont_remocoes << "\n";
    *saida << "COMPARACOES=" << sim.metricas.cont_comparacoes
           << " COPIAS=" << sim.metricas.cont_copias << "\n";
    *saida << "ORDENACOES=" << sim.metricas.cont_ordenacoes
           << " BUSCAS=" << sim.metricas.cont_buscas << "\n";
    *saida << "REALOCACOES=" << sim.metricas.cont_realocacoes << "\n";
    
    if(!interativo){
        delete entrada;
        delete saida;
    }
    
    heap_free_prioridade(&fila);
    heap_free_mesas(&sim.mesas);
    
    return 0;
}