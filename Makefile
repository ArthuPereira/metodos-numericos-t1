# Compilador e opções
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude

# Todos os .cpp da pasta src/
SRC = $(wildcard src/*.cpp)

# Nome do executável
BIN = main

# make  ->  compila
all: $(BIN)

$(BIN): $(SRC) $(wildcard include/*.hpp)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

# make run  ->  compila (se preciso) e executa
run: $(BIN)
	./$(BIN)

# make csv  ->  gera apenas os arquivos CSV na pasta saida/
csv: $(BIN)
	./$(BIN) --csv saida

# make graficos  ->  gera os CSVs e plota os gráficos de iterações
graficos: csv
	python3 ferramentas/grafico_iteracoes.py saida

# make clean  ->  apaga o executável
clean:
	rm -f $(BIN)

.PHONY: all run csv graficos clean
