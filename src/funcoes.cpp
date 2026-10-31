#include <cmath>
#include "../include/funcoes.hpp"
#include "../include/newton_raphson.hpp"
#include "../include/newton_modificado.hpp"
#include "../include/secante.hpp"

double f(double d, double a) {
    return a * std::exp(d) - 4.0 * d * d;
}

double df(double d, double a) {
    return a * std::exp(d) - 8.0 * d;
}

void finalizar(Resultado& r) {
    r.residuo   = std::fabs(f(r.d, r.a));
    r.amplifica = r.d > LIMITE_AMPLIFICACAO;
}

std::vector<Resultado> executar_metodos(double a, double d0, double e) {
    double d1 = (d0 == D0_PADRAO) ? D1_SECANTE : (d0 + 0.1);
    return {
        newton_raphson(a, d0, e),
        newton_modificado(a, d0, e),
        secante(a, d0, d1, e)
    };
}

std::vector<Resultado> executar_metodos(double a, double e) {
    return executar_metodos(a, D0_PADRAO, e);
}

std::vector<Resultado> teste_padrao() {
    return executar_metodos(A_PADRAO, E_PADRAO);
}