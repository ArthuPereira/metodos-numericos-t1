#include "../include/newton_raphson.hpp"
#include <cmath>

Resultado newton_raphson(double a, double d0, double e, int itmax) {
    Resultado r;
    r.metodo = "Newton-Raphson";
    r.a = a;

    double d = d0;

    for (int k = 0; k < itmax; ++k) {
        double fd  = f(d, a);
        double dfd = df(d, a);

        // teste para derivada nula ou muito pequena para evitar divisão por zero
        if (std::fabs(dfd) < 1e-14) {
            r.d = d;
            r.iteracoes = k;
            r.convergiu = false;
            r.erro_parada = 0.0;
            finalizar(r);
            return r;
        }

        // calculo do xk e teste do erro
        double d_novo = d - fd / dfd;
        double erro = std::fabs(d_novo - d);

        // Verifica convergência
        if (erro < e) {
            r.d = d_novo;
            r.iteracoes = k + 1;
            r.erro_parada = erro;
            r.convergiu = true;
            finalizar(r);
            return r;
        }

        d = d_novo;
    }

    // Retorno no caso onde não convergiu dentro do número máximo de iterações
    r.d = d;
    r.iteracoes = itmax;
    r.erro_parada = 0.0;
    r.convergiu = false;
    finalizar(r);
    return r;
}