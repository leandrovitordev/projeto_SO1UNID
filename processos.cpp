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

using Matriz = vector<vector<int>>;

struct ProcessData {
    int id;
    int inicio;
    int fim;
};

// lê matriz do arquivo
Matriz lerMatriz(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        throw runtime_error("erro ao abrir arquivo: " + nomeArquivo);
    }

    int linhas, colunas;
    arquivo >> linhas >> colunas;

    if (linhas <= 0 || colunas <= 0) {
        throw runtime_error("dimensoes invalidas em: " + nomeArquivo);
    }

    Matriz matriz(linhas, vector<int>(colunas, 0));

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            arquivo >> matriz[i][j]; // leitura normal
        }
    }

    return matriz;
}

// salva resultado do processo
void salvarMatriz(const string& nomeArquivo, const Matriz& matriz, double tempo) {
    ofstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "erro ao abrir arquivo: " << nomeArquivo << endl;
        return;
    }

    int linhas = matriz.size();
    int colunas = (linhas > 0) ? matriz[0].size() : 0;

    arquivo << linhas << " " << colunas << "\n";

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            arquivo << matriz[i][j];

            if (j + 1 < colunas) {
                arquivo << " ";
            }
        }
        arquivo << "\n";
    }

    arquivo << "Tempo total (ms): " << tempo << "\n";
}

// multiplicação de um bloco
void multiplicarParte(
    const Matriz& m1,
    const Matriz& m2,
    Matriz& resultado,
    int inicio,
    int base
) {
    int colunas = m2[0].size();
    int comum = m2.size();

    for (int i = inicio; i < base; i++) {
        int iLocal = i - inicio;

        for (int j = 0; j < colunas; j++) {
            int soma = 0;

            for (int k = 0; k < comum; k++) {
                soma += m1[i][k] * m2[k][j];
            }

            resultado[iLocal][j] = soma;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "uso: ./processos matriz1.txt matriz2.txt T" << endl;
        return 1;
    }

    try {
        Matriz m1 = lerMatriz(argv[1]);
        Matriz m2 = lerMatriz(argv[2]);
        int total = stoi(argv[3]);

        if (total <= 0) {
            cerr << "erro: numero de processos invalido" << endl;
            return 1;
        }

        int n1 = m1.size();
        int m1c = m1[0].size();
        int n2 = m2.size();
        int m2c = m2[0].size();

        // checando se dá pra multiplicar
        if (m1c != n2) {
            cerr << "erro: matrizes incompativeis" << endl;
            return 1;
        }

        // evita processo sem trabalho
        if (total > n1) {
            total = n1;
        }

        vector<ProcessData> dados(total);
        vector<pid_t> pids(total, -1);
        vector<string> arquivosTempo(total);

        int base = n1 / total;
        int resto = n1 % total;
        int atual = 0;

        for (int i = 0; i < total; i++) {
            int extra = (i < resto) ? 1 : 0;

            dados[i].id = i;
            dados[i].inicio = atual;
            dados[i].fim = atual + base + extra;

            arquivosTempo[i] = "tempo_processo_" + to_string(i) + ".txt";

            atual = dados[i].fim;
        }

        for (int i = 0; i < total; i++) {
            pid_t pid = fork();

            if (pid < 0) {
                cerr << "erro ao criar processo" << endl;
                return 1;
            }

            if (pid == 0) {
                // filho calcula só sua parte
                int linhas = dados[i].fim - dados[i].inicio;

                Matriz resultado(linhas, vector<int>(m2c, 0));

                auto inicio = chrono::high_resolution_clock::now();

                multiplicarParte(m1, m2, resultado, dados[i].inicio, dados[i].fim);

                auto fim = chrono::high_resolution_clock::now();

                double tempo_ms_ = chrono::duration<double, milli>(fim - inicio).count();

                string nome = "resultado_processo_" + to_string(dados[i].id) + ".txt";
                salvarMatriz(nome, resultado, tempo_ms_);

                cout << "Tempo total (ms): " << tempo_ms_ << endl;

                ofstream arqTempo(arquivosTempo[i]);
                if (!arqTempo.is_open()) {
                    cerr << "erro ao abrir arquivo: " << arquivosTempo[i] << endl;
                    _exit(1);
                }

                arqTempo << tempo_ms_ << "\n";
                arqTempo.flush();
                arqTempo.close();

                _exit(0);
            }

            pids[i] = pid;
        }

        // espera todos os filhos
        for (int i = 0; i < total; i++) {
            int status = 0;
            waitpid(pids[i], &status, 0);

            if (!WIFEXITED(status)) {
                cerr << "erro: processo nao terminou direito" << endl;
                return 1;
            }
        }

        double tempoTotal = 0;

        // pega o maior tempo
        for (int i = 0; i < total; i++) {
            ifstream arqTempo(arquivosTempo[i]);
            double tempo = 0;

            if (arqTempo.is_open()) {
                arqTempo >> tempo;
            }

            if (tempo > tempoTotal) {
                tempoTotal = tempo;
            }
        }

        cout << "processos finalizados" << endl;
        cout << "Tempo total (ms): " << tempoTotal << endl;

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
