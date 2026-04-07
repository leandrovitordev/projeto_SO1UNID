#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

using matriz_ = vector<vector<int>>;

matriz_ gerar_matriz_aleatoria_(int linhas_, int colunas_) {
    // Esta funcao cria uma matriz com dimensoes informadas; neste caso os valores serao aleatorios;
    matriz_ matriz_(linhas_, vector<int>(colunas_, 0));

    for (int i_ = 0; i_ < linhas_; ++i_) {
        for (int j_ = 0; j_ < colunas_; ++j_) {
            // O valor aleatorio fica entre 0 e 9; Isso faz com que a visualizacao fique mais simples;
            matriz_[i_][j_] = rand() % 10;
        }
    }

    return matriz_;
}

void salvar_matriz_(const string& nome_arquivo_, const matriz_& matriz_saida_) {
    // Esta funcao grava a matriz em arquivo texto; neste caso a primeira linha guarda as dimensoes;
    ofstream arquivo_(nome_arquivo_);

    if (!arquivo_.is_open()) {
        // Se o arquivo nao abrir, exibimos erro imediatamente; Isso faz com que o usuario saiba o problema;
        cerr << "Erro ao abrir o arquivo: " << nome_arquivo_ << endl;
        return;
    }

    int linhas_ = static_cast<int>(matriz_saida_.size());
    int colunas_ = (linhas_ > 0) ? static_cast<int>(matriz_saida_[0].size()) : 0;

    arquivo_ << linhas_ << " " << colunas_ << "\n";

    for (int i_ = 0; i_ < linhas_; ++i_) {
        for (int j_ = 0; j_ < colunas_; ++j_) {
            // Escrevemos cada elemento separado por espaco; neste caso o formato fica facil de ler depois;
            arquivo_ << matriz_saida_[i_][j_];

            if (j_ + 1 < colunas_) {
                arquivo_ << " ";
            }
        }

        arquivo_ << "\n";
    }

    arquivo_.close();
}

int main(int argc_, char* argv_[]) {
    // O programa precisa de quatro argumentos obrigatorios; Isso faz com que as dimensoes sejam definidas externamente;
    if (argc_ != 5) {
        cerr << "Uso: ./auxiliar n1 m1 n2 m2" << endl;
        return 1;
    }

    int n1_ = atoi(argv_[1]);
    int m1_ = atoi(argv_[2]);
    int n2_ = atoi(argv_[3]);
    int m2_ = atoi(argv_[4]);

    // Validamos se as dimensoes sao positivas; neste caso evitamos criar matrizes invalidas;
    if (n1_ <= 0 || m1_ <= 0 || n2_ <= 0 || m2_ <= 0) {
        cerr << "Erro: todas as dimensoes devem ser positivas." << endl;
        return 1;
    }

    // Inicializamos a semente pseudoaleatoria uma unica vez; Isso faz com que as matrizes variem a cada execucao;
    srand(static_cast<unsigned int>(time(nullptr)));

    matriz_ matriz1_ = gerar_matriz_aleatoria_(n1_, m1_);
    matriz_ matriz2_ = gerar_matriz_aleatoria_(n2_, m2_);

    salvar_matriz_("matriz1.txt", matriz1_);
    salvar_matriz_("matriz2.txt", matriz2_);

    cout << "Arquivos matriz1.txt e matriz2.txt gerados com sucesso." << endl;

    return 0;
}
