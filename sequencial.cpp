#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

using Matriz = vector<vector<int>>;

// lê a matriz do arquivo
Matriz lerMatriz(const string& nomeArquivo) {
    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        throw runtime_error("erro ao abrir arquivo: " + nomeArquivo);
    }

    int linhas = 0;
    int colunas = 0;
    arquivo >> linhas >> colunas;

    if (linhas <= 0 || colunas <= 0) {
        throw runtime_error("dimensoes invalidas em: " + nomeArquivo);
    }

    Matriz matriz(linhas, vector<int>(colunas, 0));

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            arquivo >> matriz[i][j]; // leitura direta msm
        }
    }

    arquivo.close();
    return matriz;
}

// salva resultado no arquivo
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
    arquivo.close();
}

// faz a multiplicação (parte ou tudo)
void multiplicarParte(
    const Matriz& m1,
    const Matriz& m2,
    Matriz& resultado,
    int linhaInicio,
    int linhaFim
) {
    int colunas = m2[0].size();
    int comum = m2.size();

    for (int i = linhaInicio; i < linhaFim; i++) {
        for (int j = 0; j < colunas; j++) {
            int soma = 0;

            for (int k = 0; k < comum; k++) {
                soma += m1[i][k] * m2[k][j]; // multiplicação padrão
            }

            resultado[i][j] = soma;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "uso: ./sequencial matriz1.txt matriz2.txt" << endl;
        return 1;
    }

    try {
        Matriz matriz1 = lerMatriz(argv[1]);
        Matriz matriz2 = lerMatriz(argv[2]);

        int n1 = matriz1.size();
        int m1 = matriz1[0].size();
        int n2 = matriz2.size();
        int m2 = matriz2[0].size();

        // verifica se dá pra multiplicar
        if (m1 != n2) {
            cerr << "erro: matrizes incompativeis" << endl;
            return 1;
        }

        Matriz resultado(n1, vector<int>(m2, 0));

        auto inicio = chrono::high_resolution_clock::now();

        // aqui roda tudo sequencial mesmo
        multiplicarParte(matriz1, matriz2, resultado, 0, n1);

        auto fim = chrono::high_resolution_clock::now();

        double tempo = chrono::duration<double, milli>(fim - inicio).count();

        salvarMatriz("resultado_sequencial.txt", resultado, tempo);

        cout << "resultado salvo em resultado_sequencial.txt" << endl;
        cout << "Tempo total (ms): " << tempo << endl;

    } catch (const exception& erro) {
        cerr << erro.what() << endl;
        return 1;
    }

    return 0;
}