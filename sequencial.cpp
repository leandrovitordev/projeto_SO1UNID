#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

using matriz_ = vector<vector<int>>;

matriz_ ler_matriz_(const string& nome_arquivo_) {
    // Esta funcao le a matriz a partir de um arquivo texto; neste caso esperamos dimensoes na primeira linha;
    ifstream arquivo_(nome_arquivo_);

    if (!arquivo_.is_open()) {
        // Se o arquivo nao abrir, encerramos com excecao logica; Isso faz com que o erro seja percebido rapidamente;
        throw runtime_error("Erro ao abrir o arquivo: " + nome_arquivo_);
    }

    int linhas_ = 0;
    int colunas_ = 0;
    arquivo_ >> linhas_ >> colunas_;

    if (linhas_ <= 0 || colunas_ <= 0) {
        // Uma matriz com dimensoes invalidas nao deve ser aceita; neste caso protegemos a execucao;
        throw runtime_error("Dimensoes invalidas no arquivo: " + nome_arquivo_);
    }

    matriz_ matriz_lida_(linhas_, vector<int>(colunas_, 0));

    for (int i_ = 0; i_ < linhas_; ++i_) {
        for (int j_ = 0; j_ < colunas_; ++j_) {
            // Lemos cada elemento exatamente na posicao correspondente; Isso faz com que a matriz preserve sua estrutura;
            arquivo_ >> matriz_lida_[i_][j_];
        }
    }

    arquivo_.close();
    return matriz_lida_;
}

void salvar_matriz_(const string& nome_arquivo_, const matriz_& matriz_saida_, double tempo_total_) {
    // Esta funcao grava a matriz resultado e o tempo final; neste caso o tempo vai ao final do arquivo;
    ofstream arquivo_(nome_arquivo_);

    if (!arquivo_.is_open()) {
        cerr << "Erro ao abrir o arquivo: " << nome_arquivo_ << endl;
        return;
    }

    int linhas_ = static_cast<int>(matriz_saida_.size());
    int colunas_ = (linhas_ > 0) ? static_cast<int>(matriz_saida_[0].size()) : 0;

    arquivo_ << linhas_ << " " << colunas_ << "\n";

    for (int i_ = 0; i_ < linhas_; ++i_) {
        for (int j_ = 0; j_ < colunas_; ++j_) {
            // Escrevemos cada valor da matriz resultado; Isso faz com que o arquivo possa ser reutilizado facilmente;
            arquivo_ << matriz_saida_[i_][j_];

            if (j_ + 1 < colunas_) {
                arquivo_ << " ";
            }
        }

        arquivo_ << "\n";
    }

    arquivo_ << "Tempo total (ms): " << tempo_total_ << "\n";
    arquivo_.close();
}

void multiplicar_parte_(
    const matriz_& matriz1_,
    const matriz_& matriz2_,
    matriz_& resultado_,
    int linha_inicial_,
    int linha_final_
) {
    // Esta funcao calcula apenas um intervalo de linhas; neste caso ela tambem serve para reuso nos outros programas;
    int colunas_resultado_ = static_cast<int>(matriz2_[0].size());
    int dimensao_interna_ = static_cast<int>(matriz2_.size());

    for (int i_ = linha_inicial_; i_ < linha_final_; ++i_) {
        for (int j_ = 0; j_ < colunas_resultado_; ++j_) {
            int soma_ = 0;

            for (int k_ = 0; k_ < dimensao_interna_; ++k_) {
                // Aqui ocorre a multiplicacao classica linha por coluna; Isso faz com que o resultado siga a definicao matematica;
                soma_ += matriz1_[i_][k_] * matriz2_[k_][j_];
            }

            resultado_[i_][j_] = soma_;
        }
    }
}

int main(int argc_, char* argv_[]) {
    // O programa sequencial exige dois arquivos de entrada; neste caso usamos exatamente os nomes passados pelo usuario;
    if (argc_ != 3) {
        cerr << "Uso: ./sequencial matriz1.txt matriz2.txt" << endl;
        return 1;
    }

    try {
        matriz_ matriz1_ = ler_matriz_(argv_[1]);
        matriz_ matriz2_ = ler_matriz_(argv_[2]);

        int n1_ = static_cast<int>(matriz1_.size());
        int m1_ = static_cast<int>(matriz1_[0].size());
        int n2_ = static_cast<int>(matriz2_.size());
        int m2_ = static_cast<int>(matriz2_[0].size());

        // Antes da multiplicacao, validamos compatibilidade; Isso faz com que nao haja acesso indevido a memoria;
        if (m1_ != n2_) {
            cerr << "Erro: numero de colunas da primeira matriz deve ser igual ao numero de linhas da segunda matriz." << endl;
            return 1;
        }

        matriz_ resultado_(n1_, vector<int>(m2_, 0));

        auto inicio_ = chrono::high_resolution_clock::now();
        multiplicar_parte_(matriz1_, matriz2_, resultado_, 0, n1_);
        auto fim_ = chrono::high_resolution_clock::now();

        double tempo_total_ = chrono::duration<double, milli>(fim_ - inicio_).count();

        salvar_matriz_("resultado_sequencial.txt", resultado_, tempo_total_);

        cout << "Resultado salvo em resultado_sequencial.txt" << endl;
        cout << "Tempo total (ms): " << tempo_total_ << endl;
    } catch (const exception& erro_) {
        // Qualquer falha de leitura ou formato chega aqui; neste caso a mensagem e mostrada ao usuario;
        cerr << erro_.what() << endl;
        return 1;
    }

    return 0;
}
