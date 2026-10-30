#include "../include/newton_modificado.hpp"

namespace manual {
    static double abs(double x) {
        return (x < 0.0) ? -x : x;
    }
}

static const double LIMITE_DIVERGENCIA = 1e15;

Resultado newton_modificado(double a, double d0, double e, int itmax) {
    Resultado r;
    r.metodo = "Newton Modificado";
    r.a = a;
    r.d = d0;

    // a derivada é calculada uma única vez, em d0, e fica fixa em todas as iterações
    const double dfd0 = df(d0, a);

    if (manual::abs(dfd0) < 1e-14) {
        r.iteracoes = 0;
        r.erro_parada = 0.0;
        r.convergiu = false;
        finalizar(r);
        return r;
    }

    double d = d0;
    double erro = 0.0;

    for (int k = 0; k < itmax; ++k) {
        double d_novo = d - f(d, a) / dfd0;

        if (d_novo != d_novo || manual::abs(d_novo) > LIMITE_DIVERGENCIA) {
            r.d = d;
            r.iteracoes = k;
            r.erro_parada = erro;
            r.convergiu = false;
            finalizar(r);
            return r;
        }

        erro = manual::abs(d_novo - d);
        d = d_novo;

        if (erro < e && manual::abs(f(d, a)) < e) {
            r.d = d;
            r.iteracoes = k + 1;
            r.erro_parada = erro;
            r.convergiu = true;
            finalizar(r);
            return r;
        }
    }
    r.d = d;
    r.iteracoes = itmax;
    r.erro_parada = erro;
    r.convergiu = false;
    finalizar(r);
    return r;
}
