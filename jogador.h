#ifndef JOGADOR_H
#define JOGADOR_H

#include "raylib.h"

typedef struct {
    int atual;
    int maxima;
} Vida;

// --- AJUSTE AQUI: Adicionei a palavra Jogador após struct ---
typedef struct Jogador {
    float x;
    float y;
    float largura;
    float altura;
    float velocidadeY;
    bool atacando;
    int tempoAtaque;
    Vida vida;
    Texture2D sprite; // png
} Jogador;

void DesenharHUD(Jogador jogador);
void ReceberDano(Jogador *jogador, int dano);

#endif
