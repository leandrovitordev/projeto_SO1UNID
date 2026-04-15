# Multiplicação de Matrizes: Sequencial vs Threads vs Processos

Este projeto foi desenvolvido com o objetivo de comparar o desempenho da multiplicação de matrizes em três abordagens diferentes: execução sequencial, uso de threads e uso de processos. A ideia principal foi observar, na prática, como o paralelismo pode influenciar o tempo de execução de uma tarefa computacionalmente pesada, especialmente quando o tamanho das matrizes cresce.

Ao longo do projeto, a proposta foi medir tempos, repetir execuções, organizar os resultados em arquivo CSV e depois gerar gráficos para visualizar melhor o comportamento de cada abordagem. Mais do que apenas implementar a multiplicação, o foco foi entender quando o paralelismo ajuda, quando ele perde eficiência e como o custo de gerenciamento de threads e processos também pesa no resultado final.

## Tecnologias utilizadas

- `C++` para implementar os algoritmos principais de multiplicação de matrizes.
- `std::thread` para a versão paralela com threads.
- `fork()` para a versão paralela com processos.
- `Bash` para automatizar a compilação, os testes e a coleta dos tempos.
- `Python` com `matplotlib` para gerar os gráficos a partir dos resultados obtidos.

## Estrutura do projeto

Os arquivos principais do projeto são:

- `auxiliar.cpp`: gera duas matrizes aleatórias e salva em `matriz1.txt` e `matriz2.txt`.
- `sequencial.cpp`: realiza a multiplicação de matrizes de forma sequencial.
- `threads.cpp`: divide a multiplicação entre várias threads.
- `processos.cpp`: divide a multiplicação entre vários processos criados com `fork()`.
- `run_final.sh`: compila os programas, executa vários testes automaticamente e gera o arquivo `resultados.csv`.
- `graficos.py`: gera os gráficos comparando os tempos obtidos nas execuções.
- `matriz1.txt` e `matriz2.txt`: arquivos de entrada com as matrizes.
- `resultados.csv`: arquivo consolidado com as médias dos tempos medidos.
- `resultado_sequencial.txt`, `resultado_thread_*.txt` e `resultado_processo_*.txt`: arquivos com resultados gerados por cada abordagem.
- `tempo_processo_*.txt`: arquivos auxiliares usados para registrar os tempos dos processos.

## Como compilar

Os programas podem ser compilados manualmente com os seguintes comandos:

```bash
g++ auxiliar.cpp -o auxiliar
g++ sequencial.cpp -o sequencial
g++ threads.cpp -o threads -pthread
g++ processos.cpp -o processos
```

Se a ideia for rodar o experimento completo, o próprio script `run_final.sh` já faz essa compilação automaticamente.

## Como executar

Antes de executar qualquer versão, é possível gerar matrizes de teste com:

```bash
./auxiliar 100 100 100 100
```

Esse comando cria duas matrizes compatíveis para multiplicação.

### Execução sequencial

```bash
./sequencial matriz1.txt matriz2.txt
```

Essa versão executa toda a multiplicação em um único fluxo, sem paralelismo.

### Execução com threads

```bash
./threads matriz1.txt matriz2.txt 4
```

No exemplo acima, a multiplicação é dividida entre 4 threads. O número de threads pode ser alterado conforme o teste desejado.

### Execução com processos

```bash
./processos matriz1.txt matriz2.txt 4
```

Nesse caso, o trabalho é dividido entre 4 processos. Cada processo calcula uma parte da matriz resultado e registra seu próprio tempo.

## Script automatizado

O arquivo `run_final.sh` foi criado para automatizar o experimento completo. Ele:

- compila todos os programas;
- define tamanhos de matrizes para teste;
- executa várias repetições para cada cenário;
- calcula médias de tempo;
- salva tudo no arquivo `resultados.csv`.

Para executar:

```bash
bash run_final.sh
```

No script, os testes são feitos com matrizes de tamanhos `100`, `200`, `400`, `800` e `1500`, repetindo cada experimento 10 vezes para reduzir oscilações.

## Geração de gráficos

Depois de coletar os resultados, o arquivo `graficos.py` pode ser usado para transformar os dados em gráficos comparativos.

Execução:

```bash
python3 graficos.py
```

O script gera as imagens:

- `grafico_threads.png`
- `grafico_processos.png`
- `grafico_comparacao.png`

Esses gráficos ajudam bastante a enxergar qual abordagem teve melhor comportamento em cada tamanho de entrada.

## Resumo dos resultados

Pelos dados definidos no script de gráficos, o comportamento geral mostra que o paralelismo trouxe ganhos importantes em vários cenários, principalmente quando a divisão da carga ficou mais equilibrada. Em tamanhos menores, a diferença entre as abordagens existe, mas não é tão grande. Já em tamanhos maiores, o impacto do paralelismo aparece de forma mais clara.

Entre as execuções com threads, a configuração com 4 threads se mostrou uma das mais consistentes. Em vários testes, ela apresentou tempos menores do que a execução sequencial e também melhor estabilidade do que usar uma quantidade muito alta de threads. Isso faz sentido porque paralelizar demais também pode introduzir sobrecarga.

Na versão com processos, também houve bons resultados em alguns cenários, especialmente com 4 processos. Por outro lado, em entradas maiores, ficou mais evidente que criar e sincronizar muitos processos pode custar caro. Em alguns casos, aumentar demais a quantidade de processos piorou o tempo em vez de melhorar.

## Conclusão

Este projeto mostrou, na prática, que paralelismo não significa automaticamente melhor desempenho em qualquer situação. O ganho depende do tamanho do problema, da forma como a tarefa é dividida e do custo adicional para gerenciar threads ou processos.

De maneira geral, a versão sequencial funciona como base de comparação, enquanto as versões paralelas mostram que é possível reduzir o tempo de execução quando a estratégia é bem ajustada. Ao mesmo tempo, os resultados também deixam claro que exagerar na quantidade de threads ou processos pode gerar o efeito contrário.

No fim, a principal aprendizagem deste trabalho foi entender que desempenho não depende só de "fazer em paralelo", mas sim de escolher uma abordagem coerente com o tipo de problema e com os recursos disponíveis no sistema.
