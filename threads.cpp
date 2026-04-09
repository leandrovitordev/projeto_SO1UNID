#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using namespace std;

using Matriz = vector<vector<int>>;

struct ThreadData {
    int id;
    int inicio;
    int fim;
    double tempo;
    Matriz resultado;
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

// salva resultado da thread
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

// multiplicação de um pedaço da matriz
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

// função da thread
void executarThread(const Matriz& m1, const Matriz& m2, ThreadData& dados) {
    int linhas = dados.fim - dados.inicio;
    int colunas = m2[0].size();

    dados.resultado = Matriz(linhas, vector<int>(colunas, 0));

    auto inicio = chrono::high_resolution_clock::now();

    multiplicarParte(m1, m2, dados.resultado, dados.inicio, dados.fim);

    auto fim = chrono::high_resolution_clock::now();

    dados.tempo = chrono::duration<double, milli>(fim - inicio).count();

    // salva resultado dessa thread
    string nome = "resultado_thread_" + to_string(dados.id) + ".txt";
    salvarMatriz(nome, dados.resultado, dados.tempo);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "uso: ./threads matriz1.txt matriz2.txt T" << endl;
        return 1;
    }

    try {
        Matriz m1 = lerMatriz(argv[1]);
        Matriz m2 = lerMatriz(argv[2]);
        int totalThreads = stoi(argv[3]);

        if (totalThreads <= 0) {
            cerr << "erro: numero de threads invalido" << endl;
            return 1;
        }

        int n1 = m1.size();
        int m1c = m1[0].size();
        int n2 = m2.size();

        // checando compatibilidade
        if (m1c != n2) {
            cerr << "erro: matrizes incompativeis" << endl;
            return 1;
        }

        // evita criar thread sem trabalho
        if (totalThreads > n1) {
            totalThreads = n1;
        }

        vector<thread> threads;
        vector<ThreadData> dados(totalThreads);

        int base = n1 / totalThreads;
        int resto = n1 % totalThreads;
        int atual = 0;

        for (int i = 0; i < totalThreads; i++) {
            int extra = (i < resto) ? 1 : 0;

            dados[i].id = i;
            dados[i].inicio = atual;
            dados[i].fim = atual + base + extra;
            dados[i].tempo = 0;

            // cada thread pega um pedaço da matriz
            threads.emplace_back(executarThread, cref(m1), cref(m2), ref(dados[i]));

            atual = dados[i].fim;
        }

        for (auto& t : threads) {
            t.join(); // espera terminar tudo
        }

        double tempoTotal = 0;

        // pega o maior tempo (thread mais lenta)
        for (const auto& d : dados) {
            if (d.tempo > tempoTotal) {
                tempoTotal = d.tempo;
            }
        }

        cout << "threads finalizadas" << endl;
        cout << "Tempo total (ms): " << tempoTotal << endl;

    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}