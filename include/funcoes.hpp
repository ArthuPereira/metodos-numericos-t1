#ifndef FUNCOES_HPP
#define FUNCOES_HPP

#include <string>
#include <vector>

constexpr double LIMITE_AMPLIFICACAO = 0.7;

// parâmetros do item d), definidos uma única vez
constexpr double A_PADRAO   = 1.0;
constexpr double D0_PADRAO  = 0.5;
constexpr double D1_SECANTE = 0.6;
constexpr double E_PADRAO   = 1e-4;

struct Resultado {
    std::string metodo;
    double a = 0;
    double d = 0;            // raiz encontrada
    double erro_parada = 0;  // |x_{k+1} - x_k| da última iteração
    double residuo = 0;      // |f(d)|
    int iteracoes = 0;
    bool convergiu = false;
    bool amplifica = false;  // d > LIMITE_AMPLIFICACAO
};

// marcação da função do movimento físico
double f(double d, double a);

// marcação da derivada
double df(double d, double a);

// chamar ao final de cada método antes de do retorno
// preenche |f(d)| e verifica se amplifica ou não
void finalizar(Resultado& r);

// roda os 3 métodos para um valor de 'a' e devolve {Newton, Modificado, Secante}
std::vector<Resultado> executar_metodos(double a, double e);
std::vector<Resultado> executar_metodos(double a, double d0, double e);

// item d): roda os métodos com valores padrão
std::vector<Resultado> teste_padrao();

#endif