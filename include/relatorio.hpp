#ifndef RELATORIO_HPP
#define RELATORIO_HPP

#include <string>
#include <vector>
#include "funcoes.hpp"

// item e): quadro resposta com d, erro e status de amplificação para cada 'a' e método
void quadro_resposta(const std::vector<std::vector<Resultado>>& resultados);

// item f): quadro comparativo com todos os dados de cada método
void quadro_comparativo(const std::vector<std::vector<Resultado>>& resultados);

// item g): análise do efeito da variação de 'a' sobre cada método e sobre o deslocamento d
void analise_variacao_a(const std::vector<std::vector<Resultado>>& resultados);

// gera os CSVs de parâmetros, histórico de iterações, resumo e varreduras (usados nos gráficos)
void gerar_csvs(const std::string& pasta = "saida",
                double a = A_PADRAO,
                double d0 = D0_PADRAO,
                double e = E_PADRAO);

// exporta os resultados de uma execução (múltiplos valores de 'a') em CSV
void exportar_resultados_csv(const std::vector<std::vector<Resultado>>& resultados,
                             const std::string& pasta = "saida");

#endif
