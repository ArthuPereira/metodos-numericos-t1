#ifndef RELATORIO_HPP
#define RELATORIO_HPP

#include <vector>
#include "funcoes.hpp"

void quadro_resposta(const std::vector<std::vector<Resultado>>& resultados);

void quadro_comparativo(const std::vector<std::vector<Resultado>>& resultados);

void analise_variacao_a(const std::vector<std::vector<Resultado>>& resultados);

#endif