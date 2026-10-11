#include "../include/funcoes.hpp"
#include <cmath>

double f(double d, double a) {
    return a * std::exp(d) - 4.0 * d * d;
}

double df(double d, double a) {
    return a * std::exp(d) - 8.0 * d;
}