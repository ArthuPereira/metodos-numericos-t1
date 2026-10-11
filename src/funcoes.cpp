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

std::vector<Resultado> executar_metodos(double a, double e) {
    return {
        newton_raphson(a, D0_PADRAO, e),
        newton_modificado(a, D0_PADRAO, e),
        secante(a, D0_PADRAO, D1_SECANTE, e)
    };
}

std::vector<Resultado> teste_padrao() {
    return executar_metodos(A_PADRAO, E_PADRAO);
}