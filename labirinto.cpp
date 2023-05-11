#include <iomanip>
#include <fstream>
#include <list>
#include <algorithm>
#include "noh.h"
#include "labirinto.h"

using namespace std;

/* ***************** */
/* CLASSE CELULA     */
/* ***************** */

string estadoCel2string(EstadoCel E)
{
    switch(E)
    {
        case EstadoCel::LIVRE:
            return "  ";
        case EstadoCel::OBSTACULO:
            return "##";
        case EstadoCel::ORIGEM:
            return "Or";
        case EstadoCel::DESTINO:
            return "De";
        case EstadoCel::CAMINHO:
            return "..";
        case EstadoCel::INVALIDO:
        default:
            break;
    }
    return "??";
}

/* ***************** */
/* CLASSE LABIRINTO  */
/* ***************** */

/// Construtores

/// Default (labirinto vazio)
Labirinto::Labirinto(): NLin(0), NCol(0), mapa(), orig(), dest() {}

/// Cria um mapa com dimensoes dadas
/// numL e numC sao as dimensoes do labirinto
Labirinto::Labirinto(int numL, int numC)
{
    gerar(numL, numC);
}

/// Cria um mapa com o conteudo do arquivo nome_arq
/// Caso nao consiga ler do arquivo, cria mapa vazio
Labirinto::Labirinto(const string& nome_arq)
{
    ler(nome_arq);
}

/// Destrutor (nao eh obrigatorio...)
Labirinto::~Labirinto() {}

/// Torna o mapa vazio
void Labirinto::clear()
{
    // Esvazia o mapa de qualquer conteudo anterior
    NLin = NCol = 0;
    mapa.clear();
    // Apaga a origem e destino do caminho
    orig = dest = Coord();
}

/// Limpa o caminho anterior
void Labirinto::limpaCaminho()
{
    if (!empty()) for (int i=0; i<NLin; ++i) for (int j=0; j<NCol; ++j)
            {
                if (at(i,j) == EstadoCel::CAMINHO)
                {
                    set(i,j, EstadoCel::LIVRE);
                }
            }
}

/// Funcoes de consulta
int Labirinto::getNumLin() const
{
    return NLin;
}

int Labirinto::getNumCol() const
{
    return NCol;
}

Coord Labirinto::getOrig() const
{
    return orig;
}

Coord Labirinto::getDest() const
{
    return dest;
}

/// Funcao de consulta
/// Retorna o estado da celula correspondente ao i-j-esimo elemento do mapa
EstadoCel Labirinto::at(int i, int j) const
{
    if (i<0 || i>=NLin || j<0 || j>=NCol)
    {
        cerr << "Coordenadas invalidas para o labirinto" << endl;
        return EstadoCel::INVALIDO;
    }
    return mapa.at(NCol*i+j);
}

/// Retorna o estado da celula C
EstadoCel Labirinto::at(const Coord& C) const
{
    return at(C.lin, C.col);
}

/// Funcao set de alteracao de valor
void Labirinto::set(int i, int j, EstadoCel valor)
{
    if (i<0 || i>=NLin || j<0 || j>=NCol)
    {
        cerr << "Coordenadas invalidas para o labirinto" << endl;
        return;
    }
    if (valor == EstadoCel::INVALIDO)
    {
        cerr << "Valor invalido para celula" << endl;
        return;
    }
    mapa.at(NCol*i+j) = valor;
}

void Labirinto::set(const Coord& C, EstadoCel valor)
{
    set(C.lin, C.col, valor);
}

/// Testa se um mapa estah vazio
bool Labirinto::empty() const
{
    return mapa.empty();
}

/// Testa se um mapa tem origem e destino definidos
bool Labirinto::origDestDefinidos() const
{
    return celulaValidaLivre(orig) && celulaValidaLivre(dest);
}

/// Testa se uma coordenada de celula eh valida para os limites de um mapa
bool Labirinto::coordValida(const Coord& C) const
{
    if (!C.valida()) return false; // Testa valores negativos
    if (C.lin >= NLin || C.col >= NCol) return false;
    return true;
}

/// Testa se uma celula eh valida e estah livre (nao eh obstaculo) em um mapa
bool Labirinto::celulaValidaLivre(const Coord& C) const
{
    if (!coordValida(C)) return false;
    if (at(C) == EstadoCel::OBSTACULO) return false;
    return true;
}

/// Testa se um movimento MovDe->MovPara eh valido
bool Labirinto::movimentoValido(const Coord& MovDe, const Coord& MovPara) const
{
    // Soh pode mover de e para celulas validas e livres
    if (!celulaValidaLivre(MovDe)) return false;
//    if (!celulaValidaLivre(MovPara)) return false;
    if (!celulaValidaLivre(MovPara)) return false;
    // Soh pode mover para celulas vizinhas, ou seja, a diferenca absoluta
    // na coordenada tanto da linha quanto da coluna eh no maximo 1
    Coord delta=abs(MovPara-MovDe);
    if (delta.lin>1 || delta.col>1) return false;

    // Nao pode mover em diagonal se colidir com alguma quina
    // Se o movimento nao for diagonal, esses testes sempre dao certo,
    // pois jah testou que MovDe e MovPara estao livres e ou a linha ou a
    // coluna de MovDe e MovPara sao iguais
    if (!celulaValidaLivre(Coord(MovDe.lin,MovPara.col))) return false;
    if (!celulaValidaLivre(Coord(MovPara.lin,MovDe.col))) return false;

    // Movimento valido
    return true;
}

/// Fixa a origem do caminho a ser encontrado
bool Labirinto::setOrigem(const Coord& C)
{
    if (!celulaValidaLivre(C)) return false;
    // Se for a mesma origen nao faz nada
    if (C==orig) return true;

    limpaCaminho();

    // Apaga a origem anterior no mapa, caso esteja definida
    if (coordValida(orig)) set(orig, EstadoCel::LIVRE);

    // Fixa a nova origem
    orig = C;
    // Marca a nova origem no mapa
    set(orig, EstadoCel::ORIGEM);

    return true;
}

/// Fixa o destino do caminho a ser encontrado
bool Labirinto::setDestino(const Coord& C)
{
    if (!celulaValidaLivre(C)) return false;
    // Se for o mesmo destino nao faz nada
    if (C==dest) return true;

    limpaCaminho();

    // Apaga o destino anterior no mapa, caso esteja definido
    if (coordValida(dest)) set(dest, EstadoCel::LIVRE);

    // Fixa o novo destino
    dest = C;
    // Marca o novo destino no mapa
    set(dest, EstadoCel::DESTINO);

    return true;
}

/// Imprime o mapa no console
void Labirinto::imprimir() const
{
    if (empty())
    {
        cout << "+------------+" << endl;
        cout << "| MAPA VAZIO |" << endl;
        cout << "+------------+" << endl;
        return;
    }

    int i,j;

    // Impressao do cabecalho
    cout << "    ";
    for (j=0; j<NCol; ++j)
    {
        cout << setfill('0') << setw(2) << j << setfill(' ') << setw(0) << ' ' ;
    }
    cout << endl;

    cout << "   +";
    for (j=0; j<NCol; ++j) cout << "--+" ;
    cout << endl;

    // Imprime as linhas
    for (i=0; i<NLin; ++i)
    {
        cout << setfill('0') << setw(2) << i << setfill(' ') << setw(0) << " |" ;
        for (j=0; j<NCol; ++j)
        {
            cout << estadoCel2string(at(i,j)) << '|' ;
        }
        cout << endl;

        cout << "   +";
        for (j=0; j<NCol; ++j) cout << "--+" ;
        cout << endl;
    }
}

/// Leh um mapa do arquivo nome_arq
/// Caso nao consiga ler do arquivo, cria mapa vazio
/// Retorna true em caso de leitura bem sucedida
bool Labirinto::ler(const string& nome_arq)
{
    // Limpa o mapa
    clear();

    // Abre o arquivo
    ifstream arq(nome_arq);

    // Resultado logico da leitura
    bool resultado=true;

    try
    {
        if (!arq.is_open()) throw 1;

        string prov;
        int numL, numC;
        int valor;

        // Leh o cabecalho
        arq >> prov >> numL >> numC;
        if (!arq.good() || prov != "LABIRINTO" ||
            numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
            numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA) throw 2;

        // Redimensiona o mapa
        NLin = numL;
        NCol = numC;
        mapa.resize(NLin*NCol);

        // Leh as celulas do arquivo
        for (int i=0; i<NLin; ++i)
            for (int j=0; j<NCol; ++j)
            {
                arq >> valor;
                if (!arq.good()) throw 3;

                if (valor == 0) set(i,j, EstadoCel::OBSTACULO);
                else set(i,j, EstadoCel::LIVRE);
            }
    }
    catch (int i)
    {
        resultado = false;
    }
    if (arq.is_open()) arq.close();
    return resultado;
}

/// Salva um mapa no arquivo nome_arq
/// Retorna true em caso de escrita bem sucedida
bool Labirinto::salvar(const string& nome_arq) const
{
    // Testa o mapa
    if (empty()) return false;

    // Abre o arquivo
    ofstream arq(nome_arq);
    if (!arq.is_open())
    {
        return false;
    }

    // Salva o cabecalho
    arq << "LABIRINTO " << NLin << ' ' << NCol << endl;

    // Salva as celulas do mapa
    for (int i=0; i<NLin; ++i)
    {
        for (int j=0; j<NCol; ++j)
        {
            if (at(i,j) == EstadoCel::OBSTACULO) arq << 0;
            else arq << 1;
            arq << ' ';
        }
        arq << endl;
    }

    arq.close();
    return true;
}

/// Gera um novo mapa aleatorio
/// numL e numC sao as dimensoes do labirinto
/// perc_obst eh o percentual de casas ocupadas no mapa. Se <=0, assume um valor aleatorio
/// entre PERC_MIN_OBST e PERC_MAX_OBST
/// Se os parametros forem incorretos, gera um mapa vazio
/// Retorna true em caso de geracao bem sucedida (parametros corretos)
bool Labirinto::gerar(int numL, int numC, double perc_obst)
{
    // Limpa o mapa
    clear();

    // Inicializa a semente de geracao de numeros aleatorios
    srand(time(nullptr));

    // Calcula o percentual de obstaculos no mapa
    if (perc_obst <= 0.0)
    {
        perc_obst = PERC_MIN_OBST +
                    (PERC_MAX_OBST-PERC_MIN_OBST)*(rand()/double(RAND_MAX));
    }

    // Testa os parametros
    if (numL<ALTURA_MIN_MAPA || numL>ALTURA_MAX_MAPA ||
        numC<LARGURA_MIN_MAPA || numC>LARGURA_MAX_MAPA ||
        perc_obst<PERC_MIN_OBST || perc_obst>PERC_MAX_OBST)
    {
        return false;
    }

    // Assume as dimensoes passadas como parametro
    NLin = numL;
    NCol = numC;

    // Redimensiona o mapa
    mapa.resize(NLin*NCol);

    // Preenche o mapa
    bool obstaculo;
    for (int i=0; i<NLin; ++i) for (int j=0; j<NCol; ++j)
        {
            obstaculo = (rand()/double(RAND_MAX) <= perc_obst);
            if (obstaculo) set(i,j, EstadoCel::OBSTACULO);
            else set(i,j, EstadoCel::LIVRE);
        }
    return true;
}

class maior_que
{
private:
    Noh noh;
public:
    maior_que(const Noh N);
    bool operator()(const Noh N) const;
};
maior_que::maior_que(Noh N): noh(N){}
bool maior_que::operator()(const Noh N) const
{
    return N.f()>noh.f();
}
/// Calcula o caminho entre a origem e o destino do labirinto usando o algoritmo A*
///
/// Retorna o comprimento do caminho (<0 se nao existe)
///
/// O parametro prof deve conter o numero de nos (profundidade) do caminho encontrado
/// ou <0 caso nao exista caminho.
///
/// O parametro NAbert deve conter o numero de nos em aberto ao termino do algoritmo A*
/// O parametro NFech deve conter o numero de nos em fechado ao termino do algoritmo A*
/// Mesmo quando nao existe caminho, esses parametros devem ter valor atribuido.
double Labirinto::calculaCaminho(int& prof, int& NAbert, int& NFech)
{
    double compr;

    if (empty() || !origDestDefinidos())
    {
        // Impossivel executar o algoritmo
        compr = -1.0;
        prof = -1;
        NAbert = NFech = -1;
        return compr;
    }
    limpaCaminho();
    if (orig==dest)
    {
        // Caminho tem comprimento e profundidade nula
        compr = 0.0;
        prof = 0;
        // Algoritmo de busca nao gerou nenhum Noh
        NAbert = NFech = 0;
        // Caminho tem comprimento nulo
        return compr;
    }

    Noh atual;
    atual.pos = atual.ant = orig;
    atual.g = 0.0;
    atual.h = atual.heuristica(dest);
    list<Noh> aberto;
    list<Noh> fechado;
    NAbert = NFech = 0;

    aberto.push_back(atual);
    Coord dir, dir_ant, dir_viz, prox, viz;
    do{
        atual = aberto.front();
        aberto.pop_front();
        fechado.push_back(atual);
        if (atual.pos != dest)
        {
            dir_ant = atual.pos - atual.ant;
            for(dir.lin = -1; dir.lin <=1; ++dir.lin )
            {
                for (dir.col = -1; dir.col <= 1; ++dir.col)
                {
                    if(dir!= Coord (0,0))
                    {
                        prox = atual.pos + dir;

                        // Testa se pode mover de atual para próximo
                        if(movimentoValido(atual.pos, prox)){
                            Noh suc;

                            //Gera novo sucessor
                            suc.pos = prox;
                            suc.ant = atual.pos;
                            suc.g = atual.g + norm(dir);
                            suc.h = suc.heuristica(dest);

                            // Custo diferencial por mudança de direção
                            if(dir != dir_ant && dir_ant != Coord (0,0))
                            {
                                suc.g += 0.001;
                            }

                            // Custo diferencial por vizinhança de obstáculo
                            for(dir_viz.lin = -1; dir_viz.lin <=1; ++dir_viz.lin ) {
                                for (dir_viz.col = -1; dir_viz.col <= 1; ++dir_viz.col) {
                                    if (dir_viz != Coord(0, 0)) {
                                        viz = suc.pos + dir_viz;
                                        if(!celulaValidaLivre(viz)){
                                           suc.g += 0.0001;
                                        }
                                    }
                                }
                            }

                            // Testa se ja foi gerado um nó igual ao sucessor
                            bool jah_existe = false;
                            list<Noh>::iterator old = find(aberto.begin(), aberto.end(), suc);
                            if(old!=aberto.end()){
                                if(suc.f() < old->f()){
                                    aberto.erase(old);
                                }
                                else{
                                    jah_existe = true;
                                }
                            }
                            else
                            {
                                old = find(fechado.begin(), fechado.end(), suc);
                                if(old!=fechado.end()){
                                    if(suc.f() < old->f()){
                                        fechado.erase(old);
                                    }
                                    else{
                                        jah_existe = true;
                                    }
                                }
                            }
                            if(!jah_existe){
                                list<Noh>::iterator big = find_if(aberto.begin(), aberto.end(), maior_que(suc));
                                aberto.insert(big, suc);
                            }
                        }
                    }
//
                }
            }
        }
    }
    while(!aberto.empty() && atual.pos!=dest);
    NAbert = aberto.size();
    NFech = fechado.size();

    if(atual.pos != dest){
        compr = -1.0;
        prof = -1;
    }
    else {
        compr = atual.g;
        prof = 1;
        while(atual.ant != orig && prof >= 0)
        {
            list<Noh>::iterator pai = find(fechado.begin(), fechado.end(), Noh (atual.ant));
            set(atual.ant,EstadoCel::CAMINHO);
            if(pai!=fechado.end()){
                atual = *pai;
                prof++;
            }
            else{
                prof = -1;
            }
        }
    }
    return compr;
}
