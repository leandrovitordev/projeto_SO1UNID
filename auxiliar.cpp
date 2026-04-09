#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

using Matriz = vector<vector<int>>;

// gera uma matriz com valores aleatorios (0 a 9)
Matriz gerarMatriz(int linhas, int colunas) {
    Matriz matriz(linhas, vector<int>(colunas, 0));

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            matriz[i][j] = rand() % 10; // numeros pequenos pra facilitar testes
        }
    }

    return matriz;
}

// salva a matriz em arquivo
void salvarMatriz(const string& nomeArquivo, const Matriz& matriz) {
    ofstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        cerr << "erro ao abrir arquivo: " << nomeArquivo << endl;
        return;
    }

    int linhas = matriz.size();
    int colunas = (linhas > 0) ? matriz[0].size() : 0;

    // primeira linha tem dimensoes
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

    arquivo.close();
}

int main(int argc, char* argv[]) {
    // precisa passar 4 valores
    if (argc != 5) {
        cerr << "uso: ./auxiliar n1 m1 n2 m2" << endl;
        return 1;
    }

    int n1 = atoi(argv[1]);
    int m1 = atoi(argv[2]);
    int n2 = atoi(argv[3]);
    int m2 = atoi(argv[4]);

    // checando se os valores fazem sentido
    if (n1 <= 0 || m1 <= 0 || n2 <= 0 || m2 <= 0) {
        cerr << "erro: dimensoes invalidas" << endl;
        return 1;
    }

    srand(time(nullptr)); // so pra variar os numeros

    Matriz matriz1 = gerarMatriz(n1, m1);
    Matriz matriz2 = gerarMatriz(n2, m2);

    salvarMatriz("matriz1.txt", matriz1);
    salvarMatriz("matriz2.txt", matriz2);

    cout << "matrizes geradas (matriz1.txt e matriz2.txt)" << endl;

    return 0;
}