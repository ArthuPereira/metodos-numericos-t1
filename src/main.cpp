#include "../include/funcoes.hpp"
#include "../include/relatorio.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

namespace {

void imprimir_uso(const char* prog) {
    std::cout << "Uso:\n"
              << "  " << prog << " [--d0 <valor>]                  Executa o teste padrão e lê testes modificados\n"
              << "  " << prog << " --padrao                        Executa apenas o teste padrão (itens d, e, f, g)\n"
              << "  " << prog << " --csv [pasta] [a] [d0] [e]      Gera apenas os arquivos CSV (padrão: saida)\n";
}

bool ler_d0_opcional_interativo(double& d0_atual) {
    if (!isatty(STDIN_FILENO)) {
        return false;
    }

    // descarta apenas o '\n' restante da leitura anterior de 'e'
    if (std::cin.peek() == '\n') {
        std::cin.get();
    }

    std::cout << "Aproximação inicial d0 [Enter para padrão = " << d0_atual << "]: ";
    std::string linha;
    if (!std::getline(std::cin, linha)) {
        return false;
    }

    std::istringstream iss(linha);
    double novo_d0 = 0.0;
    if (iss >> novo_d0) {
        d0_atual = novo_d0;
        return true;
    }
    return false;
}

} // namespace

int main(int argc, char** argv) {
    double d0_custom = D0_PADRAO;
    bool d0_via_cli  = false;
    bool apenas_padrao = false;

    // tratamento de argumentos de linha de comando
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            imprimir_uso(argv[0]);
            return 0;
        }
        if (arg == "--csv") {
            std::string pasta = (i + 1 < argc) ? argv[i + 1] : "saida";
            double a  = (i + 2 < argc) ? std::atof(argv[i + 2]) : A_PADRAO;
            double d0 = (i + 3 < argc) ? std::atof(argv[i + 3]) : D0_PADRAO;
            double e  = (i + 4 < argc) ? std::atof(argv[i + 4]) : E_PADRAO;
            gerar_csvs(pasta, a, d0, e);
            return 0;
        }
        if (arg == "--padrao") {
            apenas_padrao = true;
        } else if (arg == "--d0" && (i + 1 < argc)) {
            d0_custom  = std::atof(argv[++i]);
            d0_via_cli = true;
        }
    }

    std::cout << "  MOVIMENTO FÍSICO: f(d) = a * e^d - 4 * d^2   (Limite seguro: d <= 0.7 cm)\n"
              << "  Métodos: Newton-Raphson | Newton Modificado | Secante Tradicional\n";

    std::cout << "\n>>> [PARTE 1] TESTE PADRÃO (a = " << A_PADRAO
              << ", d0 = " << D0_PADRAO
              << ", d1_secante = " << D1_SECANTE
              << ", e = " << E_PADRAO << ")\n";

    std::vector<std::vector<Resultado>> res_padrao = { teste_padrao() };
    quadro_resposta(res_padrao);
    quadro_comparativo(res_padrao);
    gerar_csvs("saida", A_PADRAO, D0_PADRAO, E_PADRAO);

    if (apenas_padrao) {
        analise_variacao_a(res_padrao);
        return 0;
    }

    // Testes Modificados — Entrada: n, valores de 'a' e precisão 'e'
    std::cout << "\n>>> [PARTE 2] TESTES MODIFICADOS (ENTRADA PERSONALIZADA)\n";
    if (isatty(STDIN_FILENO)) {
        std::cout << "Informe o número de valores de 'a' (n) [0 para encerrar]: ";
    }

    int n = 0;
    if (!(std::cin >> n) || n <= 0) {
        std::cout << "\nNenhum teste adicional informado. Exibindo análise geral de variação de 'a':\n";
        analise_variacao_a(res_padrao);
        return 0;
    }

    std::vector<double> valores_a(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        if (isatty(STDIN_FILENO)) {
            std::cout << "  Valor de a[" << (i + 1) << "/" << n << "]: ";
        }
        if (!(std::cin >> valores_a[static_cast<std::size_t>(i)])) {
            std::cerr << "Erro: entrada inválida ao ler os valores de 'a'.\n";
            return 1;
        }
    }

    double e = E_PADRAO;
    while (true) {
        if (isatty(STDIN_FILENO)) {
            std::cout << "Informe a precisão (e > 0) [ex: 1e-4]: ";
        }
        if (!(std::cin >> e)) {
            std::cerr << "Erro: entrada inválida ao ler a precisão 'e'.\n";
            return 1;
        }
        if (e > 0.0) {
            break;
        }
        std::cerr << "A precisão 'e' deve ser estritamente positiva.\n";
        if (!isatty(STDIN_FILENO)) {
            return 1;
        }
    }

    if (!d0_via_cli) {
        ler_d0_opcional_interativo(d0_custom);
    }

    double d1_usado = (d0_custom == D0_PADRAO) ? D1_SECANTE : (d0_custom + 0.1);
    std::cout << "\nExecutando " << n << " teste(s) com d0 = " << d0_custom
              << " (d1_secante = " << d1_usado << ") e precisão e = " << e << "...\n";

    std::vector<std::vector<Resultado>> resultados;
    resultados.reserve(static_cast<std::size_t>(n));
    for (double ai : valores_a) {
        resultados.push_back(executar_metodos(ai, d0_custom, e));
    }

    quadro_resposta(resultados);
    quadro_comparativo(resultados);
    analise_variacao_a(resultados);
    exportar_resultados_csv(resultados, "saida");

    // se foi testado um único valor de 'a' personalizado, atualiza também o CSV de histórico/resumo se desejado
    if (n == 1) {
        gerar_csvs("saida", valores_a[0], d0_custom, e);
    }

    return 0;
}
