#!/usr/bin/env python3
"""
Usa os CSVs gerados por src/relatorio.cpp (só precisa de parametros.csv e historico.csv).

requisitos:  pip install matplotlib numpy

Passo 00 (compilar e gerar CSVs):
make csv
(ou: ./main --csv saida [a] [d0] [e])

Passo 01 (gerar imagens):
python3 ferramentas/grafico_iteracoes.py saida
"""
import csv
import math
import os
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")  # só salva imagem, não abre janela
import matplotlib.pyplot as plt
import numpy as np

PASTA = sys.argv[1] if len(sys.argv) > 1 else "saida"
METODOS = {"Newton-Raphson": "iteracoes_newton_raphson.png",
           "Newton Modificado": "iteracoes_newton_modificado.png",
           "Secante": "iteracoes_secante.png"}
AZUL, VERMELHO = "#2d2de0", "#e0393e"   # cores do slide: curva azul, retas vermelhas
ZOOM_A_PARTIR = 8    # se o método tem pelo menos essa quantidade de iterações, faz o zoom perto da raiz
DPI = 170


def garantir_csvs():
    """Cria a pasta de saída e gera parametros.csv/historico.csv via ./main caso ainda não existam."""
    os.makedirs(PASTA, exist_ok=True)
    necessarios = [os.path.join(PASTA, "parametros.csv"), os.path.join(PASTA, "historico.csv")]
    if all(os.path.isfile(arq) for arq in necessarios):
        return

    raiz_repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    bin_main = os.path.join(raiz_repo, "main")
    if not os.path.isfile(bin_main):
        subprocess.run(["make"], cwd=raiz_repo, check=True)
    subprocess.run([bin_main, "--csv", PASTA], cwd=raiz_repo, check=True)


def le(nome):
    with open(os.path.join(PASTA, nome), newline="") as f:
        return [{k: (v if k == "metodo" else float(v)) for k, v in row.items()} for row in csv.DictReader(f)]


garantir_csvs()
par = le("parametros.csv")[0]
A, D0, D1, E = par["a"], par["d0"], par["d1"], par["e"]
hist = {m: [r for r in le("historico.csv") if r["metodo"] == m] for m in METODOS}


def f(d):
    return A * math.exp(d) - 4 * d * d


def g(x, casas=6):
    return f"{x:.{casas}f}".replace(".", ",")


def sub(k):
    """índice com subscrito unicode: 12 -> ₁₂"""
    return "".join("₀₁₂₃₄₅₆₇₈₉"[int(c)] for c in str(k))


def pontos(m):
    """x0, (x1 na secante), e o d depois de cada iteração"""
    return [D0] + ([D1] if m == "Secante" else []) + [r["d"] for r in hist[m]]


# raiz de referência: onde o Newton-Raphson terminou
RAIZ = hist["Newton-Raphson"][-1]["d"]


def desenha(ax, m, xs, xlo, xhi, rotulos, estende_primeira, fonte=10, desde=0, sep=0.05):
    """Curva, retas dos passos, verticais tracejadas, pontos e rótulos dentro de [xlo, xhi].
    desde: primeiro passo (reta vermelha) a desenhar;  sep: distância mínima entre rótulos (fração da largura)."""
    fx = [f(x) for x in xs]
    curva_x = np.linspace(xlo, xhi, 500)
    ax.plot(curva_x, [f(x) for x in curva_x], color=AZUL, lw=1.6, zorder=2)

    # reta do primeiro passo estendida pelo gráfico inteiro (como a tangente do slide)
    if estende_primeira:
        # pela reta que passa por (x_i0, f) e (x_i1, f) (secante) ou por (x0, f0) e (x1, 0) (newton)
        if m == "Secante":
            xa, ya, xb, yb = xs[0], fx[0], xs[1], fx[1]
        else:
            xa, ya, xb, yb = xs[0], fx[0], xs[1], 0.0
        inclinacao = (yb - ya) / (xb - xa)
        ax.plot([xlo, xhi], [ya + inclinacao * (xlo - xa), ya + inclinacao * (xhi - xa)],
                color=VERMELHO, lw=0.9, alpha=0.45, zorder=1)

    # retas de cada passo: terminam no eixo, no próximo ponto
    primeiro = 1 if m == "Secante" else 0
    for i in range(max(primeiro, desde), len(xs) - 1):
        xa, ya = (xs[i - 1], fx[i - 1]) if m == "Secante" else (xs[i], fx[i])
        ax.plot([xa, xs[i + 1]], [ya, 0.0], color=VERMELHO, lw=1.3, zorder=3)

    # verticais tracejadas e pontos sobre a curva
    for x, y in zip(xs, fx):
        ax.plot([x, x], [0, y], color="black", ls=(0, (2, 2)), lw=1.0, zorder=3)
        ax.plot(x, y, "o", color="black", ms=3.2, zorder=4)

    # raiz
    ax.plot(RAIZ, 0, "o", mfc="white", mec="black", ms=5.5, zorder=5)
    ax.annotate("ξ", (RAIZ, 0), xytext=(0, 30), textcoords="offset points", ha="center",
                fontsize=fonte + 2, fontweight="bold",
                arrowprops=dict(arrowstyle="-", color="gray", lw=0.8, shrinkA=2, shrinkB=3))

    ultimo = {True: None, False: None}
    for k in rotulos:
        x = xs[k]
        if x < xlo or x > xhi:
            continue
        cima = x < RAIZ
        if ultimo[cima] is not None and abs(x - ultimo[cima]) < sep * (xhi - xlo):
            continue
        ultimo[cima] = x
        ax.annotate(f"d{sub(k)}", (x, 0), xytext=(0, 9 if cima else -9 - fonte),
                    textcoords="offset points", ha="center", fontsize=fonte, zorder=6,
                    bbox=dict(fc="white", ec="none", pad=0.6, alpha=0.85))


def eixos_estilo_slide(ax, xlo, xhi, ylo, yhi):
    ax.set_xlim(xlo, xhi)
    ax.set_ylim(ylo, yhi)
    ax.axis("off")
    seta = dict(arrowstyle="-|>", color="black", lw=1.2, shrinkA=0, shrinkB=0)
    ax.annotate("", xy=(xhi, 0), xytext=(xlo, 0), arrowprops=seta, annotation_clip=False)
    ax.annotate("", xy=(xlo, yhi), xytext=(xlo, ylo), arrowprops=seta, annotation_clip=False)
    ax.text(xhi, 0, " d", ha="left", va="center", fontsize=11, style="italic")


def figura(m):
    xs = pontos(m)
    n_iter = len(hist[m])

    todos = [x for mm in METODOS for x in pontos(mm)]
    xlo, xhi = min(todos) - 0.05, max(todos) + 0.06
    ys = [f(x) for x in np.linspace(xlo, xhi, 300)]
    folga = 0.10 * (max(ys) - min(ys))
    ylo, yhi = min(ys) - folga, max(ys) + folga

    fig, ax = plt.subplots(figsize=(9.5, 6.2))
    eixos_estilo_slide(ax, xlo, xhi, ylo, yhi)
    ax.text(xhi, f(xhi) + 0.05 * (yhi - ylo), "f(d)", color=AZUL, ha="right", fontsize=12, fontweight="bold")

    desenha(ax, m, xs, xlo, xhi, range(min(len(xs), 6)), estende_primeira=True)   # rotula só d0..d5

    # zoom perto da raiz, para ver o final da aproximação quando há muitas iterações
    if n_iter >= ZOOM_A_PARTIR:
        z = xs[3:]
        pad = 0.006
        zlo, zhi = min(z + [RAIZ]) - pad, max(z + [RAIZ]) + pad
        zy = [f(x) for x in np.linspace(zlo, zhi, 100)]
        zf = 0.35 * (max(zy) - min(zy))
        ins = ax.inset_axes([0.54, 0.62, 0.44, 0.33])
        ins.set_xlim(zlo, zhi)
        ins.set_ylim(min(zy + [0]) - zf, max(zy + [0]) + zf)
        ins.set_yticks([])
        ins.set_xticks([])
        for lado in ins.spines.values():
            lado.set_color("gray")
        ins.axhline(0, color="black", lw=1.0)
        desenha(ins, m, xs, zlo, zhi, range(3, min(len(xs), 9)), estende_primeira=False, fonte=9, desde=3, sep=0.06)   # rotula d3..d8
        ins.set_title(f"ampliação perto de ξ (até d{sub(n_iter)})", fontsize=9, color="gray")
        ax.indicate_inset_zoom(ins, edgecolor="gray", alpha=0.8)

    linhas = [f"d{sub(0)} = {g(D0, 4)}"]
    if m == "Secante":
        linhas.append(f"d{sub(1)} = {g(D1, 4)}")
    base = len(linhas)
    for k, r in enumerate(hist[m], start=base):
        if k <= 5 or k == len(xs) - 1:
            linhas.append(f"d{sub(k)} = {g(r['d'])}")
        elif k == 6:
            linhas.append("   ...")
    ax.text(0.015, 0.03, "\n".join(linhas), transform=ax.transAxes, fontsize=9.5, va="bottom",
            family="monospace", bbox=dict(fc="white", ec="lightgray", alpha=0.9))

    titulo_eps = f"10$^{{{round(math.log10(E))}}}$" if abs(E - 10 ** round(math.log10(E))) < 1e-12 * E else g(E, 6)
    ax.set_title(f"{m} — {n_iter} iterações\n"
                 f"a = {f'{A:g}'.replace('.', ',')},  $d_{{0}}$ = {f'{D0:g}'.replace('.', ',')},  ε = {titulo_eps}",
                 fontsize=13, color="black")
    caminho = os.path.join(PASTA, METODOS[m])
    fig.savefig(caminho, dpi=DPI, bbox_inches="tight")
    plt.close(fig)
    print("salvo:", caminho)


if __name__ == "__main__":
    for metodo in METODOS:
        figura(metodo)
