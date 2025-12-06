#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;

#define TEMPO_ATENDIMENTO 30
#define NUM_MESAS 3

enum TipoEvento { EVT_CHEGADA, EVT_CHAMAR, EVT_FIM, EVT_INVALIDO };

struct Cliente {
    int id_cliente;
    int momento_chegada;
    int tamanho_grupo;
    int prioridade;
    int tolerancia_espera;
};

struct Fila {
    Cliente* buffer;
    int capacidade;
    int inicio;
    int fim;
    int quantidade;
    long long realocacoes;
    long long copias;
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
};

struct MinHeapMesas {
    int *livre_em;
    int *id;
    int n, cap;
};

struct Simulacao {
    int momento_atual;
    Metricas metricas;
    MinHeapMesas mesas;
};

void heap_swap(MinHeapMesas *h, int i, int j){
    int te = h->livre_em[i], ti = h->id[i];
    h->livre_em[i] = h->livre_em[j]; h->id[i] = h->id[j];
    h->livre_em[j] = te;             h->id[j] = ti;
}

void heap_init(MinHeapMesas *h, int cap){
    h->cap = (cap < 1 ? 1 : cap);
    h->n = 0;
    h->livre_em = new int[h->cap];
    h->id       = new int[h->cap];
}

void heap_free(MinHeapMesas *h){
    delete[] h->livre_em; h->livre_em = nullptr;
    delete[] h->id;       h->id = nullptr;
    h->n = h->cap = 0;
}

void heap_grow(MinHeapMesas *h){
    int nova = h->cap * 2;
    int *ne = new int[nova];
    int *ni = new int[nova];
    for(int i=0;i<h->n;++i){ ne[i]=h->livre_em[i]; ni[i]=h->id[i]; }
    delete[] h->livre_em; delete[] h->id;
    h->livre_em = ne; h->id = ni; h->cap = nova;
}

void heap_push(MinHeapMesas *h, int livre, int mesa_id){
    if(h->n == h->cap) heap_grow(h);
    int i = h->n++;
    h->livre_em[i] = livre; h->id[i] = mesa_id;
    while(i > 0){
        int p = (i - 1) / 2;
        if(h->livre_em[p] <= h->livre_em[i]) break;
        heap_swap(h, i, p); i = p;
    }
}

bool heap_peek(const MinHeapMesas *h, int *livre, int *mesa_id){
    if(h->n == 0) return false;
    *livre = h->livre_em[0]; *mesa_id = h->id[0];
    return true;
}

bool heap_pop(MinHeapMesas *h, int *livre, int *mesa_id){
    if(h->n == 0) return false;
    *livre = h->livre_em[0]; *mesa_id = h->id[0];
    h->n--;
    if(h->n > 0){
        h->livre_em[0] = h->livre_em[h->n];
        h->id[0]       = h->id[h->n];
        int i = 0;
        while(true){
            int l = 2*i+1, r = 2*i+2, m = i;
            if(l < h->n && h->livre_em[l] < h->livre_em[m]) m = l;
            if(r < h->n && h->livre_em[r] < h->livre_em[m]) m = r;
            if(m == i) break;
            heap_swap(h, i, m); i = m;
        }
    }
    return true;
}

void fila_iniciar(Fila *f, int capacidade_inicial){
    f->capacidade = (capacidade_inicial < 1 ? 4 : capacidade_inicial);
    f->buffer = new Cliente[f->capacidade];
    f->inicio = f->fim = f->quantidade = 0;
    f->realocacoes = 0;
    f->copias = 0;
}

void fila_liberar(Fila *f){
    delete[] f->buffer;
    f->buffer = nullptr;
    f->capacidade = f->inicio = f->fim = f->quantidade = 0;
    f->realocacoes = f->copias = 0;
}

bool fila_vazia(const Fila *f){
    return f->quantidade == 0;
}

void fila_crescer(Fila *f){
    int nova_capacidade = f->capacidade * 2;
    Cliente *novo = new Cliente[nova_capacidade];
    for(int i = 0; i < f->quantidade; ++i){
        int idx = (f->inicio + i) % f->capacidade;
        memcpy(&novo[i], &f->buffer[idx], sizeof(Cliente));
    }
    f->copias += f->quantidade;
    delete[] f->buffer;
    f->buffer = novo;
    f->capacidade = nova_capacidade;
    f->inicio = 0;
    f->fim = f->quantidade;
    f->realocacoes++;
}

void fila_inserir(Fila *f, const Cliente *c, Metricas *m){
    if(f->quantidade == f->capacidade) fila_crescer(f);
    memcpy(&f->buffer[f->fim], c, sizeof(Cliente));
    f->fim = (f->fim + 1) % f->capacidade;
    f->quantidade++;
    if(m) m->cont_insercoes++;
}

void fila_remover(Fila *f, Cliente *destino, Metricas *m){
    if(fila_vazia(f)){ cerr << "Erro: remocao em fila vazia.\n"; exit(1); }
    memcpy(destino, &f->buffer[f->inicio], sizeof(Cliente));
    f->inicio = (f->inicio + 1) % f->capacidade;
    f->quantidade--;
    if(m) m->cont_remocoes++;
}

TipoEvento interpretar_tipo_evento(const string &tok){
    if(tok == "CHEGADA") return EVT_CHEGADA;
    if(tok == "CHAMAR")  return EVT_CHAMAR;
    if(tok == "FIM")     return EVT_FIM;
    return EVT_INVALIDO;
}

void simulacao_iniciar(Simulacao *s){
    s->momento_atual = 0;
    memset(&s->metricas, 0, sizeof(Metricas));
    heap_init(&s->mesas, NUM_MESAS);
    for(int k=0;k<NUM_MESAS;++k){
        heap_push(&s->mesas, 0, k);
    }
}

void processar_chegada(Simulacao *s, Fila *f, const Cliente *c, ostream *saida){
    if(c->momento_chegada > s->momento_atual)
        s->momento_atual = c->momento_chegada;
    *saida << s->momento_atual << ", CHEGADA, id=" << c->id_cliente
           << " grupo=" << c->tamanho_grupo
           << " prioridade=" << c->prioridade
           << " tolerancia=" << c->tolerancia_espera << "\n";
    fila_inserir(f, c, &s->metricas);
}

void processar_chamada(Simulacao *s, Fila *f, int momento_chamada, ostream *saida){
    if(momento_chamada > s->momento_atual)
        s->momento_atual = momento_chamada;
    s->metricas.cont_comparacoes++;
    while(!fila_vazia(f)){
        Cliente topo;
        fila_remover(f, &topo, &s->metricas);
        int esperou = s->momento_atual - topo.momento_chegada;
        if(esperou > topo.tolerancia_espera){
            *saida << s->momento_atual << ", CHAMADA, ABANDONO id=" << topo.id_cliente
                   << " esperou=" << esperou
                   << " > tolerancia=" << topo.tolerancia_espera << "\n";
            continue;
        } else {
            int livre_topo, mesa_id;
            if(!heap_peek(&s->mesas, &livre_topo, &mesa_id)){
                *saida << s->momento_atual << ", CHAMADA, ERRO: sem mesas configuradas\n";
                return;
            }
            if(s->momento_atual < livre_topo){
                *saida << s->momento_atual << ", CHAMADA, nenhuma mesa livre (prox_livre="
                       << livre_topo << ")\n";
                f->inicio = (f->inicio - 1 + f->capacidade) % f->capacidade;
                memcpy(&f->buffer[f->inicio], &topo, sizeof(Cliente));
                f->quantidade++;
                return;
            }
            heap_pop(&s->mesas, &livre_topo, &mesa_id);
            int inicio_atendimento = s->momento_atual;
            int tempo_espera = inicio_atendimento - topo.momento_chegada;
            s->metricas.soma_tempos_espera += tempo_espera;
            s->metricas.total_atendidos++;
            int termina = inicio_atendimento + TEMPO_ATENDIMENTO;
            heap_push(&s->mesas, termina, mesa_id);
            *saida << s->momento_atual << ", CHAMADA, id=" << topo.id_cliente
                   << " mesa=" << mesa_id
                   << " inicia_atendimento; espera=" << tempo_espera
                   << "; termina_em=" << termina << "\n";
            return;
        }
    }
    *saida << s->momento_atual << ", CHAMADA, fila_vazia\n";
}

bool salvar_metricas_bin(const char *caminho, const Simulacao *s, const Fila *f){
    ofstream fp(caminho, ios::binary);
    if(!fp) return false;
    Metricas m;
    memcpy(&m, &s->metricas, sizeof(Metricas));
    m.tamanho_fila_restante = f->quantidade;
    m.cont_realocacoes = f->realocacoes;
    m.cont_copias = f->copias;
    fp.write(reinterpret_cast<const char*>(&m), sizeof(Metricas));
    return fp.good();
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    istream *entrada = nullptr;
    ostream *saida = nullptr;
    string caminho_binario;
    bool interativo = (argc == 1) || (argc >= 2 && string(argv[1]) == "-");
    if(interativo){
        entrada = &cin;
        saida = &cout;
    } else {
        if(argc < 3){
            cerr << "Uso: " << argv[0] << " input.in output.txt [metricas.bin]\n";
            return 1;
        }
        ifstream *in = new ifstream(argv[1]);
        if(!*in){ cerr << "Erro ao abrir '" << argv[1] << "'.\n"; return 1; }
        ofstream *out = new ofstream(argv[2]);
        if(!*out){ cerr << "Erro ao criar '" << argv[2] << "'.\n"; return 1; }
        entrada = in;
        saida = out;
        if(argc >= 4) caminho_binario = argv[3];
    }
    Fila fila; fila_iniciar(&fila, 8);
    Simulacao sim; simulacao_iniciar(&sim);
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
            if(!(iss >> c.id_cliente >> c.momento_chegada >> c.tamanho_grupo >> c.prioridade >> c.tolerancia_espera)){
                cerr << "Linha " << numero_linha << " invalida (CHEGADA).\n";
                continue;
            }
            processar_chegada(&sim, &fila, &c, saida);
        } 
        else if(tipo == EVT_CHAMAR){
            int t;
            if(!(iss >> t)){ cerr << "Linha " << numero_linha << " invalida (CHAMAR).\n"; continue; }
            processar_chamada(&sim, &fila, t, saida);
        } 
        else if(tipo == EVT_FIM){
            encerrar = true;
            *saida << sim.momento_atual << ", FIM\n";
        } 
        else {
            cerr << "Linha " << numero_linha << ": evento desconhecido.\n";
        }
    }
    *saida << "---\n";
    *saida << "ATENDIDOS=" << sim.metricas.total_atendidos << "\n";
    if(sim.metricas.total_atendidos > 0){
        double media = (double)sim.metricas.soma_tempos_espera / (double)sim.metricas.total_atendidos;
        *saida << "AVG_ESPERA=" << fixed << setprecision(2) << media << "\n";
    } else {
        *saida << "AVG_ESPERA=NA\n";
    }
    sim.metricas.tamanho_fila_restante = fila.quantidade;
    sim.metricas.cont_realocacoes = fila.realocacoes;
    sim.metricas.cont_copias = fila.copias;
    *saida << "FILA_RESTANTE=" << fila.quantidade << "\n";
    *saida << "INSERCOES=" << sim.metricas.cont_insercoes
           << " REMOCOES=" << sim.metricas.cont_remocoes
           << " COMPARACOES=" << sim.metricas.cont_comparacoes << "\n";
    *saida << "REALOCACOES=" << fila.realocacoes
           << " COPIAS=" << fila.copias << "\n";
    if(!interativo && !caminho_binario.empty())
        salvar_metricas_bin(caminho_binario.c_str(), &sim, &fila);
    if(!interativo){
        delete &static_cast<ifstream&>(*entrada);
        delete &static_cast<ofstream&>(*saida);
    }
    heap_free(&sim.mesas);
    fila_liberar(&fila);
    return 0;
}
