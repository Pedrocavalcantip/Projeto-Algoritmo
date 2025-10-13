#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

using namespace std;

#define TEMPO_ATENDIMENTO 30

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

struct Simulacao {
    int momento_atual;
    int momento_mesa_livre;
    Metricas metricas;
};

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
    s->momento_mesa_livre = 0;
    memset(&s->metricas, 0, sizeof(Metricas));
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

    if(s->momento_atual < s->momento_mesa_livre){
        *saida << s->momento_atual << ", CHAMADA, nenhuma mesa livre (prox_livre=" 
               << s->momento_mesa_livre << ")\n";
        return;
    }

    if(fila_vazia(f)){
        *saida << s->momento_atual << ", CHAMADA, fila_vazia\n";
        return;
    }

    Cliente c;
    fila_remover(f, &c, &s->metricas);

    int inicio_atendimento = s->momento_atual;
    int tempo_espera = inicio_atendimento - c.momento_chegada;
    s->metricas.soma_tempos_espera += tempo_espera;
    s->metricas.total_atendidos++;
    s->momento_mesa_livre = inicio_atendimento + TEMPO_ATENDIMENTO;

    *saida << s->momento_atual << ", CHAMADA, id=" << c.id_cliente
           << " inicia_atendimento; espera=" << tempo_espera
           << "; termina_em=" << s->momento_mesa_livre << "\n";
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

    fila_liberar(&fila);
    return 0;
}