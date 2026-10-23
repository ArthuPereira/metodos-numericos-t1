#include "../include/secante.hpp"
#include <cmath>

// recebe o parâmetro da equação, aproximações iniciais d0 e d1, erro aceitável, máximo de iterações 
Resultado secante(double a, double d0, double d1, double e, int itmax) {
    Resultado res;
    res.metodo = "Secante";
    res.a = a;
    res.iteracoes = 0;

    // testa pra ver se d0 já não é a raiz
    double f0 = f(d0, a);
    if (std::fabs(f0) < e) {
        res.d = d0;
        res.convergiu = true;
        finalizar(res);
        return res;
    }

    // mesmo teste pro d1
    double f1 = f(d1, a);
    double erro = std::fabs(d1 - d0);
    if (std::fabs(f1) < e) {
        res.d = d1;
        res.erro_parada = erro;
        res.convergiu = true;
        finalizar(res);
        return res;
    }

    // seta as variáveis de trabalho
    double x0 = d0;
    double x1 = d1;

    // loop enquanto tiver dentro do limite de iterações
    while (res.iteracoes < itmax) {
        // evita divisão por zero se f(x1) e f(x0) tiverem muito perto
        if (std::fabs(f1 - f0) < 1e-14) {
            break;
        }

        // formula da secante
        double x2 = x1 - f1 / (f1 - f0) * (x1 - x0);

        // proteção extra pra evitar infinito ou NaN
        if (!std::isfinite(x2)) {
            break;
        }

        // incrementa as iterações, seta o erro do passo e guarda o f(x2)
        res.iteracoes++;
        erro = std::fabs(x2 - x1);
        double f2 = f(x2, a);

        // faz os testes dos critérios de parada
        if (std::fabs(f2) < e || erro < e) {
            res.d = x2;
            res.erro_parada = erro;
            res.convergiu = true;
            finalizar(res);
            return res;
        }

        // atualiza os valores pra próxima iteração
        x0 = x1;  f0 = f1;
        x1 = x2;  f1 = f2;
    }

    // se não convergiu, retorna o último x1 válido e seta a convergência como False
    res.d = x1;
    res.convergiu = false;
    res.erro_parada = erro;
    finalizar(res);
    return res;
}
