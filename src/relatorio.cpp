#include "../include/relatorio.hpp"
#include "../include/newton_raphson.hpp"
#include "../include/newton_modificado.hpp"
#include "../include/secante.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace {

constexpr int KMAX_CSV  = 60;
constexpr int ITMAX_CSV = 500;

// conta caracteres visíveis em UTF-8 para alinhar tabelas com acentos corretamente
int largura_visivel(const std::string& s) {
    int n = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) {
            ++n;
        }
    }
    return n;
}

std::string celula(const std::string& texto, int largura, bool direita = false) {
    int faltam = largura - largura_visivel(texto);
    if (faltam <= 0) {
        return texto;
    }
    std::string espacos(static_cast<std::size_t>(faltam), ' ');
    return direita ? (espacos + texto) : (texto + espacos);
}

std::string fmt_fixo(double v, int casas = 8) {
    if (!std::isfinite(v)) {
        return "NaN/Inf";
    }
    std::ostringstream oss;
    if (std::fabs(v) >= 1e4) {
        oss << std::scientific << std::setprecision(3) << v;
    } else {
        oss << std::fixed << std::setprecision(casas) << v;
    }
    return oss.str();
}

std::string fmt_cientifico(double v, int casas = 4) {
    if (!std::isfinite(v)) {
        return "NaN/Inf";
    }
    std::ostringstream oss;
    oss << std::scientific << std::setprecision(casas) << v;
    return oss.str();
}

std::string fmt_geral(double v, int digitos = 6) {
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(digitos) << v;
    return oss.str();
}

std::string status_fisico(const Resultado& r) {
    if (!r.convergiu || !std::isfinite(r.d)) {
        return "NÃO CONVERGIU";
    }
    if (r.residuo > 1e-2) {
        return "FALSA CONV. (|f| alto)";
    }
    return r.amplifica ? "AMPLIFICA (d > 0.7 cm)" : "SEGURO (d <= 0.7 cm)";
}

// wrappers uniformes para geração dos CSVs (migrado de ferramentas/gerar_csv.cpp)
Resultado roda_newton(double a, double d0, double, double e, int itmax) {
    return newton_raphson(a, d0, e, itmax);
}

Resultado roda_modificado(double a, double d0, double, double e, int itmax) {
    return newton_modificado(a, d0, e, itmax);
}

Resultado roda_secante(double a, double d0, double d1, double e, int itmax) {
    return secante(a, d0, d1, e, itmax);
}

using MetodoCSV = Resultado (*)(double a, double d0, double d1, double e, int itmax);
const MetodoCSV METODOS_CSV[3] = { roda_newton, roda_modificado, roda_secante };

double d1_para(double d0) {
    return (d0 == D0_PADRAO) ? D1_SECANTE : (d0 + 0.1);
}

FILE* abrir_csv(const std::string& pasta, const char* nome) {
    std::error_code ec;
    std::filesystem::path dir(pasta);
    if (!dir.empty()) {
        std::filesystem::create_directories(dir, ec);
    }
    std::filesystem::path caminho = dir / nome;
    FILE* fp = std::fopen(caminho.string().c_str(), "w");
    if (!fp) {
        std::fprintf(stderr, "Erro: nao foi possivel criar o arquivo %s\n", caminho.string().c_str());
    }
    return fp;
}

void escrever_linha_varredura(FILE* fp, const Resultado& r, double param) {
    std::fprintf(fp, "%s,%.12g,%.12g,%d,%.12g,%.12g,%d,%d\n",
                 r.metodo.c_str(), param, r.d, r.iteracoes, r.erro_parada,
                 r.residuo, static_cast<int>(r.convergiu), static_cast<int>(r.amplifica));
}

void imprimir_tabela_resumo_a(const std::vector<std::vector<Resultado>>& grupos) {
    const std::string sep = "+------------+----------------+-------------+-------------+-------------+-------------------------+\n";
    std::cout << sep;
    std::cout << "| " << celula("a", 10, true)
              << " | " << celula("d ref. (cm)", 14, true)
              << " | " << celula("Iter. NR", 11, true)
              << " | " << celula("Iter. Mod.", 11, true)
              << " | " << celula("Iter. Sec.", 11, true)
              << " | " << celula("Comportamento Físico", 23) << " |\n";
    std::cout << sep;

    for (const auto& grupo : grupos) {
        if (grupo.size() < 3) continue;
        const Resultado& nr  = grupo[0];
        const Resultado& mod = grupo[1];
        const Resultado& sec = grupo[2];

        auto raiz_valida = [](const Resultado& r) -> bool {
            return r.convergiu && std::isfinite(r.d) && r.residuo <= 1e-2;
        };

        auto fmt_it = [&](const Resultado& r) -> std::string {
            if (!r.convergiu) return std::to_string(r.iteracoes) + "*";
            if (!raiz_valida(r)) return std::to_string(r.iteracoes) + "!";
            return std::to_string(r.iteracoes);
        };

        // escolhe uma raiz efetivamente válida de referência para relatar d e amplificação
        const Resultado* ref = raiz_valida(nr) ? &nr : (raiz_valida(sec) ? &sec : (raiz_valida(mod) ? &mod : nullptr));
        std::string d_txt    = ref ? fmt_fixo(ref->d, 6) : "---";
        std::string fis_txt  = ref ? status_fisico(*ref) : "Sem raiz real";

        std::cout << "| " << celula(fmt_geral(nr.a, 6), 10, true)
                  << " | " << celula(d_txt, 14, true)
                  << " | " << celula(fmt_it(nr), 11, true)
                  << " | " << celula(fmt_it(mod), 11, true)
                  << " | " << celula(fmt_it(sec), 11, true)
                  << " | " << celula(fis_txt, 23) << " |\n";
    }
    std::cout << sep;
    std::cout << "  (* não convergiu | ! parou por |Δd| < e mas com |f(d)| alto)\n";
}

} // namespace

void quadro_resposta(const std::vector<std::vector<Resultado>>& resultados) {
    if (resultados.empty()) {
        std::cout << "[Aviso] Nenhum resultado para exibir no quadro resposta.\n";
        return;
    }

    const std::string sep = "+------------+--------------------+----------------+----------------+-------------------------+\n";
    std::cout << "\n============================== QUADRO RESPOSTA ==============================\n";
    std::cout << sep;
    std::cout << "| " << celula("a", 10, true)
              << " | " << celula("Método", 18)
              << " | " << celula("d (cm)", 14, true)
              << " | " << celula("Erro |Δd|", 14, true)
              << " | " << celula("Situação Física", 23) << " |\n";
    std::cout << sep;

    for (std::size_t i = 0; i < resultados.size(); ++i) {
        for (const Resultado& r : resultados[i]) {
            std::string d_str = r.convergiu ? fmt_fixo(r.d, 8) : ("(" + fmt_fixo(r.d, 4) + ")");
            std::cout << "| " << celula(fmt_geral(r.a, 6), 10, true)
                      << " | " << celula(r.metodo, 18)
                      << " | " << celula(d_str, 14, true)
                      << " | " << celula(fmt_cientifico(r.erro_parada, 4), 14, true)
                      << " | " << celula(status_fisico(r), 23) << " |\n";
        }
        std::cout << sep;
    }
}

void quadro_comparativo(const std::vector<std::vector<Resultado>>& resultados) {
    if (resultados.empty()) {
        std::cout << "[Aviso] Nenhum resultado para exibir no quadro comparativo.\n";
        return;
    }

    const std::string sep = "+----------+--------------------+--------------+--------+--------------+--------------+-----------+--------------+\n";
    std::cout << "\n========================================= QUADRO COMPARATIVO =========================================\n";
    std::cout << sep;
    std::cout << "| " << celula("a", 8, true)
              << " | " << celula("Método", 18)
              << " | " << celula("d (cm)", 12, true)
              << " | " << celula("Iter.", 6, true)
              << " | " << celula("Erro |Δd|", 12, true)
              << " | " << celula("Resíduo |f|", 12, true)
              << " | " << celula("Convergiu", 9)
              << " | " << celula("Amplifica?", 12) << " |\n";

    for (const auto& grupo : resultados) {
        std::cout << sep;
        for (const Resultado& r : grupo) {
            std::string conv_str = r.convergiu ? "Sim" : "Não";
            std::string amp_str  = (!r.convergiu || !std::isfinite(r.d))
                                       ? "---"
                                       : (r.amplifica ? "SIM (>0.7)" : "Não (<=0.7)");

            std::cout << "| " << celula(fmt_geral(r.a, 6), 8, true)
                      << " | " << celula(r.metodo, 18)
                      << " | " << celula(fmt_fixo(r.d, 8), 12, true)
                      << " | " << celula(std::to_string(r.iteracoes), 6, true)
                      << " | " << celula(fmt_cientifico(r.erro_parada, 4), 12, true)
                      << " | " << celula(fmt_cientifico(r.residuo, 4), 12, true)
                      << " | " << celula(conv_str, 9)
                      << " | " << celula(amp_str, 12) << " |\n";
        }
        std::cout << sep;

        // síntese direta entre os métodos para este valor de 'a'
        if (!grupo.empty()) {
            const Resultado* mais_rapido   = nullptr;
            const Resultado* menor_residuo = nullptr;
            double d_min =  std::numeric_limits<double>::infinity();
            double d_max = -std::numeric_limits<double>::infinity();
            int qtd_conv = 0;

            for (const Resultado& r : grupo) {
                if (r.convergiu && std::isfinite(r.d)) {
                    ++qtd_conv;
                    if (!mais_rapido || r.iteracoes < mais_rapido->iteracoes) {
                        mais_rapido = &r;
                    }
                    if (!menor_residuo || r.residuo < menor_residuo->residuo) {
                        menor_residuo = &r;
                    }
                    d_min = std::min(d_min, r.d);
                    d_max = std::max(d_max, r.d);
                }
            }

            if (qtd_conv > 0) {
                std::cout << "  -> a = " << fmt_geral(grupo[0].a, 6)
                          << ": menor nº de iterações = " << mais_rapido->metodo
                          << " (" << mais_rapido->iteracoes << " it.)"
                          << " | menor |f(d)| = " << menor_residuo->metodo
                          << " (" << fmt_cientifico(menor_residuo->residuo, 2) << ")";
                if (qtd_conv >= 2) {
                    std::cout << " | |Δd| máx entre métodos = " << fmt_cientifico(d_max - d_min, 2);
                }
                std::cout << "\n";
            } else {
                std::cout << "  -> a = " << fmt_geral(grupo[0].a, 6)
                          << ": nenhum dos métodos convergiu.\n";
            }
        }
    }
}

void analise_variacao_a(const std::vector<std::vector<Resultado>>& resultados) {
    std::cout << "\n========================= ANÁLISE DA VARIAÇÃO DE 'a' =========================\n";

    if (!resultados.empty()) {
        // ordena cópia por valor de 'a' para evidenciar a progressão
        auto ordenados = resultados;
        std::sort(ordenados.begin(), ordenados.end(),
                  [](const std::vector<Resultado>& g1, const std::vector<Resultado>& g2) {
                      if (g1.empty() || g2.empty()) return g1.size() < g2.size();
                      return g1[0].a < g2[0].a;
                  });

        std::cout << "\n1) Comportamento nos valores de 'a' informados:\n";
        imprimir_tabela_resumo_a(ordenados);

        // estatísticas por método nos valores testados
        std::cout << "\n   Desempenho agregado nos testes informados:\n";
        for (std::size_t m = 0; m < 3; ++m) {
            std::string nome;
            int conv = 0;
            int total = 0;
            int soma_it = 0;
            int min_it = std::numeric_limits<int>::max();
            int max_it = 0;

            for (const auto& g : ordenados) {
                if (m >= g.size()) continue;
                nome = g[m].metodo;
                ++total;
                if (g[m].convergiu) {
                    ++conv;
                    soma_it += g[m].iteracoes;
                    min_it = std::min(min_it, g[m].iteracoes);
                    max_it = std::max(max_it, g[m].iteracoes);
                }
            }

            if (total > 0) {
                std::cout << "   - " << celula(nome + ":", 20)
                          << " convergiu em " << conv << "/" << total << " caso(s)";
                if (conv > 0) {
                    double media = static_cast<double>(soma_it) / conv;
                    std::cout << " | iterações (mín / méd / máx): "
                              << min_it << " / " << fmt_fixo(media, 2) << " / " << max_it;
                }
                std::cout << "\n";
            }
        }
    }

    // se foram passados poucos valores de 'a', executa também pontos-chave de referência
    if (resultados.size() < 3) {
        const std::vector<double> as_ref = { -0.5, 0.0, 0.5, 0.97, 1.0, 1.5, 2.0, 2.5 };
        std::vector<std::vector<Resultado>> varredura_ref;
        varredura_ref.reserve(as_ref.size());
        for (double ai : as_ref) {
            varredura_ref.push_back(executar_metodos(ai, E_PADRAO));
        }
        std::cout << "\n2) Varredura de referência (d0 = " << D0_PADRAO
                  << ", d1 = " << D1_SECANTE << ", e = " << E_PADRAO << "):\n";
        imprimir_tabela_resumo_a(varredura_ref);
    }

    const double a_lim_amp = 4.0 * LIMITE_AMPLIFICACAO * LIMITE_AMPLIFICACAO * std::exp(-LIMITE_AMPLIFICACAO);
    const double a_max_pos = 16.0 * std::exp(-2.0);
    const double a_df0     = 4.0 * std::exp(-D0_PADRAO);

    std::cout << "\nSíntese teórica e numérica do efeito de 'a' em f(d) = a*e^d - 4*d^2:\n"
              << "  * a < 0: f(d) < 0 para todo d real -> não existe raiz real; os 3 métodos falham/divergem.\n"
              << "  * a = 0: raiz dupla em d = 0 -> Newton-Raphson e Secante perdem convergência rápida\n"
              << "    (passam a convergir linearmente, exigindo mais iterações) e Newton Modificado fica lento.\n"
              << "  * 0 < a <= " << fmt_fixo(a_lim_amp, 4) << " (4*0.7^2*e^-0.7): a raiz positiva cresce com 'a',\n"
              << "    porém mantém d <= 0.7 cm (SISTEMA SEGURO). Newton-Raphson e Secante convergem em poucas\n"
              << "    iterações; Newton Modificado exige mais passos conforme a raiz se afasta de d0.\n"
              << "  * " << fmt_fixo(a_lim_amp, 4) << " < a <= " << fmt_fixo(a_max_pos, 4)
              << " (16*e^-2): o deslocamento supera 0.7 cm (AMPLIFICA!).\n"
              << "    Perto de a = " << fmt_fixo(a_max_pos, 4) << " (d -> 2), f'(d) -> 0, encarecendo todos os métodos\n"
              << "    e podendo impedir a convergência do Newton Modificado.\n"
              << "  * a > " << fmt_fixo(a_max_pos, 4) << ": deixam de existir raízes positivas, restando apenas uma raiz\n"
              << "    negativa (d < 0). Além disso, em a ≈ " << fmt_fixo(a_df0, 4)
              << " tem-se f'(d0) = 0 para d0 = " << D0_PADRAO << ",\n"
              << "    inviabilizando ou degradando severamente o Newton Modificado.\n";
}

void gerar_csvs(const std::string& pasta, double a, double d0, double e) {
    double d1 = d1_para(d0);

    FILE* fp = abrir_csv(pasta, "parametros.csv");
    if (!fp) return;
    std::fprintf(fp, "a,d0,d1,e\n%.12g,%.12g,%.12g,%.12g\n", a, d0, d1, e);
    std::fclose(fp);

    fp = abrir_csv(pasta, "historico.csv");
    if (!fp) return;
    std::fprintf(fp, "metodo,k,d,erro_parada,residuo,convergiu\n");
    for (int m = 0; m < 3; ++m) {
        for (int k = 1; k <= KMAX_CSV; ++k) {
            Resultado r = METODOS_CSV[m](a, d0, d1, e, k);
            std::fprintf(fp, "%s,%d,%.12g,%.12g,%.12g,%d\n",
                         r.metodo.c_str(), k, r.d, r.erro_parada, r.residuo,
                         static_cast<int>(r.convergiu));
            if (r.convergiu || r.iteracoes < k) break;
        }
    }
    std::fclose(fp);

    fp = abrir_csv(pasta, "resumo.csv");
    if (!fp) return;
    std::fprintf(fp, "metodo,d,iteracoes,erro_parada,residuo,convergiu,amplifica\n");
    for (int m = 0; m < 3; ++m) {
        Resultado r = METODOS_CSV[m](a, d0, d1, e, ITMAX_CSV);
        std::fprintf(fp, "%s,%.12g,%d,%.12g,%.12g,%d,%d\n",
                     r.metodo.c_str(), r.d, r.iteracoes, r.erro_parada,
                     r.residuo, static_cast<int>(r.convergiu), static_cast<int>(r.amplifica));
    }
    std::fclose(fp);

    fp = abrir_csv(pasta, "varredura_a.csv");
    if (!fp) return;
    std::fprintf(fp, "metodo,a,d,iteracoes,erro_parada,residuo,convergiu,amplifica\n");
    for (int m = 0; m < 3; ++m) {
        for (int i = 0; i <= 200; ++i) {
            double ai = -1.0 + 0.02 * i;
            escrever_linha_varredura(fp, METODOS_CSV[m](ai, d0, d1, e, ITMAX_CSV), ai);
        }
    }
    std::fclose(fp);

    fp = abrir_csv(pasta, "varredura_d0.csv");
    if (!fp) return;
    std::fprintf(fp, "metodo,d0,d,iteracoes,erro_parada,residuo,convergiu,amplifica\n");
    for (int m = 0; m < 3; ++m) {
        for (int i = 0; i <= 400; ++i) {
            double d0i = -2.0 + 0.02 * i;
            escrever_linha_varredura(fp, METODOS_CSV[m](a, d0i, d0i + 0.1, e, ITMAX_CSV), d0i);
        }
    }
    std::fclose(fp);

    std::cout << "[CSV] Arquivos gerados em '" << pasta
              << "/' (a=" << a << ", d0=" << d0 << ", d1=" << d1 << ", e=" << e << ")\n";
}

void exportar_resultados_csv(const std::vector<std::vector<Resultado>>& resultados,
                             const std::string& pasta) {
    if (resultados.empty()) return;

    FILE* fp = abrir_csv(pasta, "resultados_execucao.csv");
    if (!fp) return;

    std::fprintf(fp, "a,metodo,d,iteracoes,erro_parada,residuo,convergiu,amplifica\n");
    for (const auto& grupo : resultados) {
        for (const Resultado& r : grupo) {
            std::fprintf(fp, "%.12g,%s,%.12g,%d,%.12g,%.12g,%d,%d\n",
                         r.a, r.metodo.c_str(), r.d, r.iteracoes, r.erro_parada,
                         r.residuo, static_cast<int>(r.convergiu), static_cast<int>(r.amplifica));
        }
    }
    std::fclose(fp);

    std::cout << "[CSV] Resultados dos testes exportados em '" << pasta
              << "/resultados_execucao.csv'\n";
}
