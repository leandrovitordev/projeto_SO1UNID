#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace std;

using matriz_ = vector<vector<int>>;

struct dados_processo_ {
    int id_;
    int linha_inicial_;
    int linha_final_;
};

matriz_ ler_matriz_(const string& nome_arquivo_) {
    // Esta funcao le a matriz salva em texto; neste caso o formato e identico ao usado nos demais programas;
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
            // Cada valor e lido conforme a ordem do arquivo; Isso faz com que a estrutura original seja preservada;
            arquivo_ >> matriz_lida_[i_][j_];
        }
    }

    arquivo_.close();
    return matriz_lida_;
}

void salvar_matriz_(const string& nome_arquivo_, const matriz_& matriz_saida_, double tempo_total_) {
    // Esta funcao grava o resultado parcial de um processo; neste caso o tempo individual tambem fica salvo;
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
            // Gravamos apenas as linhas calculadas por este processo; Isso faz com que cada arquivo seja independente;
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
    // Esta funcao reaplica a multiplicacao classica em um intervalo; neste caso o resultado vai para uma matriz parcial;
    int colunas_resultado_ = static_cast<int>(matriz2_[0].size());
    int dimensao_interna_ = static_cast<int>(matriz2_.size());

    for (int i_original_ = linha_inicial_original_; i_original_ < linha_final_original_; ++i_original_) {
        int i_local_ = i_original_ - linha_inicial_local_;

        for (int j_ = 0; j_ < colunas_resultado_; ++j_) {
            int soma_ = 0;

            for (int k_ = 0; k_ < dimensao_interna_; ++k_) {
                // A soma acumulada segue a definicao da multiplicacao matricial; Isso faz com que o bloco fique correto;
                soma_ += matriz1_[i_original_][k_] * matriz2_[k_][j_];
            }

            resultado_[i_local_][j_] = soma_;
        }
    }
}

int main(int argc_, char* argv_[]) {
    // O programa de processos tambem recebe dois arquivos e T; neste caso T representa a quantidade de processos filhos;
    if (argc_ != 4) {
        cerr << "Uso: ./processos matriz1.txt matriz2.txt T" << endl;
        return 1;
    }

    try {
        // A leitura e feita antes do fork; Isso faz com que o requisito seja obedecido e os dados ja existam na memoria;
        matriz_ matriz1_ = ler_matriz_(argv_[1]);
        matriz_ matriz2_ = ler_matriz_(argv_[2]);
        int total_processos_ = stoi(argv_[3]);

        if (total_processos_ <= 0) {
            cerr << "Erro: T deve ser maior que zero." << endl;
            return 1;
        }

        int n1_ = static_cast<int>(matriz1_.size());
        int m1_ = static_cast<int>(matriz1_[0].size());
        int n2_ = static_cast<int>(matriz2_.size());
        int m2_ = static_cast<int>(matriz2_[0].size());

        // Confirmamos se a multiplicacao e valida; neste caso a regra obrigatoria e m1 igual a n2;
        if (m1_ != n2_) {
            cerr << "Erro: numero de colunas da primeira matriz deve ser igual ao numero de linhas da segunda matriz." << endl;
            return 1;
        }

        // Limitamos a quantidade de processos ao numero de linhas; Isso faz com que nao haja processos ociosos;
        if (total_processos_ > n1_) {
            total_processos_ = n1_;
        }

        vector<dados_processo_> dados_processos_(total_processos_);
        vector<pid_t> pids_(total_processos_, -1);
        vector<string> arquivos_tempo_(total_processos_);

        int linhas_base_ = n1_ / total_processos_;
        int resto_ = n1_ % total_processos_;
        int linha_atual_ = 0;

        for (int i_ = 0; i_ < total_processos_; ++i_) {
            int extra_ = (i_ < resto_) ? 1 : 0;
            dados_processos_[i_].id_ = i_;
            dados_processos_[i_].linha_inicial_ = linha_atual_;
            dados_processos_[i_].linha_final_ = linha_atual_ + linhas_base_ + extra_;
            arquivos_tempo_[i_] = "tempo_processo_" + to_string(i_) + ".txt";
            linha_atual_ = dados_processos_[i_].linha_final_;
        }

        for (int i_ = 0; i_ < total_processos_; ++i_) {
            pid_t pid_ = fork();

            if (pid_ < 0) {
                cerr << "Erro ao criar processo." << endl;
                return 1;
            }

            if (pid_ == 0) {
                // O processo filho calcula apenas seu intervalo; neste caso tambem grava seu proprio tempo em arquivo auxiliar;
                int quantidade_linhas_ = dados_processos_[i_].linha_final_ - dados_processos_[i_].linha_inicial_;
                matriz_ resultado_parcial_(quantidade_linhas_, vector<int>(m2_, 0));

                auto inicio_ = chrono::high_resolution_clock::now();

                multiplicar_parte_(
                    matriz1_,
                    matriz2_,
                    resultado_parcial_,
                    dados_processos_[i_].linha_inicial_,
                    dados_processos_[i_].linha_inicial_,
                    dados_processos_[i_].linha_final_
                );

                auto fim_ = chrono::high_resolution_clock::now();
                double tempo_ms_ = chrono::duration<double, milli>(fim_ - inicio_).count();

                string nome_resultado_ = "resultado_processo_" + to_string(dados_processos_[i_].id_) + ".txt";
                salvar_matriz_(nome_resultado_, resultado_parcial_, tempo_ms_);

                ofstream arquivo_tempo_(arquivos_tempo_[i_]);

                if (arquivo_tempo_.is_open()) {
                    // O tempo tambem e salvo isoladamente para o pai ler depois; Isso faz com que o maximo possa ser calculado;
                    arquivo_tempo_ << tempo_ms_ << "\n";
                    arquivo_tempo_.close();
                }

                _exit(0);
            }

            pids_[i_] = pid_;
        }

        for (int i_ = 0; i_ < total_processos_; ++i_) {
            int status_ = 0;
            waitpid(pids_[i_], &status_, 0);

            if (!WIFEXITED(status_)) {
                cerr << "Erro: um processo filho nao terminou corretamente." << endl;
                return 1;
            }
        }

        double tempo_total_ = 0.0;

        for (int i_ = 0; i_ < total_processos_; ++i_) {
            ifstream arquivo_tempo_(arquivos_tempo_[i_]);
            double tempo_ms_ = 0.0;

            if (arquivo_tempo_.is_open()) {
                // O pai le os tempos produzidos pelos filhos; neste caso escolhemos o maior deles;
                arquivo_tempo_ >> tempo_ms_;
                arquivo_tempo_.close();
            }

            if (tempo_ms_ > tempo_total_) {
                tempo_total_ = tempo_ms_;
            }
        }

        cout << "Arquivos parciais dos processos gerados com sucesso." << endl;
        cout << "Tempo total (ms): " << tempo_total_ << endl;
    } catch (const exception& erro_) {
        cerr << erro_.what() << endl;
        return 1;
    }

    return 0;
}
