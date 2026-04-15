#!/bin/bash

REPETICOES=10
TAMANHOS=(100 200 400 800 1500)
THREADS_VALORES=(2 4 8 16)
PROCESSOS_VALORES=(2 4 8 16)
CSV="resultados.csv"

extrair_tempo_saida() {
    local saida="$1"
    local tempo

    tempo=$(printf '%s\n' "$saida" | grep "Tempo total (ms):" | awk '{print $4}' | tail -n 1)
    echo "${tempo:-0}"
}

maior_tempo_processos() {
    local maior

    maior=$(awk '
        BEGIN { max = 0; found = 0 }
        NF > 0 {
            valor = $1 + 0
            if (!found || valor > max) {
                max = valor
            }
            found = 1
        }
        END {
            if (found) {
                print max
            } else {
                print 0
            }
        }
    ' tempo_processo_*.txt 2>/dev/null)

    echo "${maior:-0}"
}

somar() {
    echo "$(echo "$1 + $2" | bc -l)"
}

media() {
    echo "$(echo "scale=6; $1 / $2" | bc -l)"
}

echo "Compilando arquivos..."
g++ auxiliar.cpp -o auxiliar
g++ sequencial.cpp -o sequencial
g++ threads.cpp -o threads -pthread
g++ processos.cpp -o processos

echo "tamanho,seq,th2,th4,th8,th16,pr2,pr4,pr8,pr16" > "$CSV"

for tamanho in "${TAMANHOS[@]}"; do
    echo "Rodando tamanho ${tamanho}..."

    soma_seq=0
    soma_th2=0
    soma_th4=0
    soma_th8=0
    soma_th16=0
    soma_pr2=0
    soma_pr4=0
    soma_pr8=0
    soma_pr16=0

    for ((execucao = 1; execucao <= REPETICOES; execucao++)); do
        echo "  Execucao ${execucao}/${REPETICOES}"

        ./auxiliar "$tamanho" "$tamanho" "$tamanho" "$tamanho" > /dev/null

        saida_seq=$(./sequencial matriz1.txt matriz2.txt 2>/dev/null)
        seq=$(extrair_tempo_saida "$saida_seq")
        soma_seq=$(somar "$soma_seq" "$seq")

        saida_th2=$(./threads matriz1.txt matriz2.txt 2 2>/dev/null)
        th2=$(extrair_tempo_saida "$saida_th2")
        soma_th2=$(somar "$soma_th2" "$th2")

        saida_th4=$(./threads matriz1.txt matriz2.txt 4 2>/dev/null)
        th4=$(extrair_tempo_saida "$saida_th4")
        soma_th4=$(somar "$soma_th4" "$th4")

        saida_th8=$(./threads matriz1.txt matriz2.txt 8 2>/dev/null)
        th8=$(extrair_tempo_saida "$saida_th8")
        soma_th8=$(somar "$soma_th8" "$th8")

        saida_th16=$(./threads matriz1.txt matriz2.txt 16 2>/dev/null)
        th16=$(extrair_tempo_saida "$saida_th16")
        soma_th16=$(somar "$soma_th16" "$th16")

        rm -f tempo_processo_*.txt
        ./processos matriz1.txt matriz2.txt 2 > /dev/null 2>&1
        pr2=$(maior_tempo_processos)
        soma_pr2=$(somar "$soma_pr2" "$pr2")

        rm -f tempo_processo_*.txt
        ./processos matriz1.txt matriz2.txt 4 > /dev/null 2>&1
        pr4=$(maior_tempo_processos)
        soma_pr4=$(somar "$soma_pr4" "$pr4")

        rm -f tempo_processo_*.txt
        ./processos matriz1.txt matriz2.txt 8 > /dev/null 2>&1
        pr8=$(maior_tempo_processos)
        soma_pr8=$(somar "$soma_pr8" "$pr8")

        rm -f tempo_processo_*.txt
        ./processos matriz1.txt matriz2.txt 16 > /dev/null 2>&1
        pr16=$(maior_tempo_processos)
        soma_pr16=$(somar "$soma_pr16" "$pr16")
    done

    media_seq=$(media "$soma_seq" "$REPETICOES")
    media_th2=$(media "$soma_th2" "$REPETICOES")
    media_th4=$(media "$soma_th4" "$REPETICOES")
    media_th8=$(media "$soma_th8" "$REPETICOES")
    media_th16=$(media "$soma_th16" "$REPETICOES")
    media_pr2=$(media "$soma_pr2" "$REPETICOES")
    media_pr4=$(media "$soma_pr4" "$REPETICOES")
    media_pr8=$(media "$soma_pr8" "$REPETICOES")
    media_pr16=$(media "$soma_pr16" "$REPETICOES")

    echo "${tamanho},${media_seq},${media_th2},${media_th4},${media_th8},${media_th16},${media_pr2},${media_pr4},${media_pr8},${media_pr16}" >> "$CSV"
done

echo "Experimentos finalizados. Resultados salvos em ${CSV}."
