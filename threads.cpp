#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace std;

using matriz_ = vector<vector<int>>;

struct dados_thread_ {
    int id_;
    int linha_inicial_;
    int linha_final_;
    double tempo_ms_;
    matriz_ resultado_parcial_;
};

matriz_ ler_matriz_(const string& nome_arquivo_) {
    // Esta funcao le uma matriz de arquivo; neste caso o formato e o mesmo usado pelos outros programas;
    ifstream arquivo_(nome_arquivo_);

    if (!arquivo_.is_open()) {
        throw runtime_error("Erro ao abrir o arquivo: " + nome_arquivo_);
    }

    int linhas_ = 0;
    int colunas_ = 0;
    arquivo_ >> linhas_ >> colunas_;

    if (linhas_ <= 0 || colunas_ <= 0) {
        throw runtime_error("Dimensoes invalidas no arquivo: " + nome_arquivo_);
    }

    matriz_ matriz_lida_(linhas_, vector<int>(colunas_, 0));

    for (int i_ = 0; i_ < linhas_; ++i_) {
        for (int j_ = 0; j_ < colunas_; ++j_) {
            // A leitura e sequencial e previsivel; Isso faz com que cada posicao seja preenchida corretamente;
            arquivo_ >> matriz_lida_[i_][j_];
        }
    }

    arquivo_.close();
    return matriz_lida_;
}

void salvar_matriz_(const string& nome_arquivo_, const matriz_& matriz_saida_, double tempo_total_) {
    // Esta funcao grava o resultado parcial de cada thread; neste caso tambem salvamos o tempo individual;
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
            // Gravamos somente o bloco calculado pela thread; Isso faz com que cada arquivo represente sua parcela;
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
    int linha_inicial_original_,
    int linha_inicial_local_,
    int linha_final_original_
) {
    // Esta funcao calcula um bloco de linhas dentro de uma matriz parcial; neste caso usamos indices global e local;
    int colunas_resultado_ = static_cast<int>(matriz2_[0].size());
    int dimensao_interna_ = static_cast<int>(matriz2_.size());

    for (int i_original_ = linha_inicial_original_; i_original_ < linha_final_original_; ++i_original_) {
        int i_local_ = i_original_ - linha_inicial_local_;

        for (int j_ = 0; j_ < colunas_resultado_; ++j_) {
            int soma_ = 0;

            for (int k_ = 0; k_ < dimensao_interna_; ++k_) {
                // Aplicamos a regra da multiplicacao matricial no intervalo da thread; Isso faz com que o trabalho seja dividido corretamente;
                soma_ += matriz1_[i_original_][k_] * matriz2_[k_][j_];
            }

            resultado_[i_local_][j_] = soma_;
        }
    }
}

void executar_thread_(const matriz_& matriz1_, const matriz_& matriz2_, dados_thread_& dados_) {
    // Cada thread mede apenas seu proprio trabalho; neste caso o arquivo sera salvo ao final da rotina;
    int quantidade_linhas_ = dados_.linha_final_ - dados_.linha_inicial_;
    int colunas_resultado_ = static_cast<int>(matriz2_[0].size());

    dados_.resultado_parcial_ = matriz_(quantidade_linhas_, vector<int>(colunas_resultado_, 0));

    auto inicio_ = chrono::high_resolution_clock::now();

    multiplicar_parte_(
        matriz1_,
        matriz2_,
        dados_.resultado_parcial_,
        dados_.linha_inicial_,
        dados_.linha_inicial_,
        dados_.linha_final_
    );

    auto fim_ = chrono::high_resolution_clock::now();
    dados_.tempo_ms_ = chrono::duration<double, milli>(fim_ - inicio_).count();

    string nome_arquivo_ = "resultado_thread_" + to_string(dados_.id_) + ".txt";
    salvar_matriz_(nome_arquivo_, dados_.resultado_parcial_, dados_.tempo_ms_);
}

int main(int argc_, char* argv_[]) {
    // O programa exige dois arquivos e a quantidade de threads; Isso faz com que a divisao possa ser configurada;
    if (argc_ != 4) {
        cerr << "Uso: ./threads matriz1.txt matriz2.txt T" << endl;
        return 1;
    }

    try {
        // A leitura ocorre antes da criacao das threads; neste caso seguimos exatamente o requisito solicitado;
        matriz_ matriz1_ = ler_matriz_(argv_[1]);
        matriz_ matriz2_ = ler_matriz_(argv_[2]);
        int total_threads_ = stoi(argv_[3]);

        if (total_threads_ <= 0) {
            cerr << "Erro: T deve ser maior que zero." << endl;
            return 1;
        }

        int n1_ = static_cast<int>(matriz1_.size());
        int m1_ = static_cast<int>(matriz1_[0].size());
        int n2_ = static_cast<int>(matriz2_.size());
        int m2_ = static_cast<int>(matriz2_[0].size());

        // Validamos compatibilidade matricial antes da computacao; Isso faz com que o programa falhe de forma segura;
        if (m1_ != n2_) {
            cerr << "Erro: numero de colunas da primeira matriz deve ser igual ao numero de linhas da segunda matriz." << endl;
            return 1;
        }

        // Se houver mais threads que linhas, limitamos o valor; neste caso evitamos criar trabalhadores sem carga;
        if (total_threads_ > n1_) {
            total_threads_ = n1_;
        }

        vector<thread> threads_;
        vector<dados_thread_> dados_threads_(total_threads_);

        int linhas_base_ = n1_ / total_threads_;
        int resto_ = n1_ % total_threads_;
        int linha_atual_ = 0;

        for (int i_ = 0; i_ < total_threads_; ++i_) {
            int extra_ = (i_ < resto_) ? 1 : 0;
            int linha_inicial_ = linha_atual_;
            int linha_final_ = linha_inicial_ + linhas_base_ + extra_;

            dados_threads_[i_].id_ = i_;
            dados_threads_[i_].linha_inicial_ = linha_inicial_;
            dados_threads_[i_].linha_final_ = linha_final_;
            dados_threads_[i_].tempo_ms_ = 0.0;

            // Criamos cada thread com seu intervalo exclusivo; Isso faz com que nao exista disputa por escrita no mesmo bloco;
            threads_.emplace_back(executar_thread_, cref(matriz1_), cref(matriz2_), ref(dados_threads_[i_]));

            linha_atual_ = linha_final_;
        }

        for (thread& thread_atual_ : threads_) {
            // Aguardamos o termino de todas as threads; neste caso garantimos que todos os arquivos ja foram gerados;
            thread_atual_.join();
        }

        double tempo_total_ = 0.0;

        for (const dados_thread_& dados_ : dados_threads_) {
            // O tempo total e o maior entre os tempos individuais; Isso faz com que o criterio siga a especificacao;
            if (dados_.tempo_ms_ > tempo_total_) {
                tempo_total_ = dados_.tempo_ms_;
            }
        }

        cout << "Arquivos parciais das threads gerados com sucesso." << endl;
        cout << "Tempo total (ms): " << tempo_total_ << endl;
    } catch (const exception& erro_) {
        cerr << erro_.what() << endl;
        return 1;
    }

    return 0;
}
