#ifndef JOGADOR_H
#define JOGADOR_H

#include "raylib.h"

typedef struct
{
    int atual;
    int maxima;

} Vida;

typedef struct
{
    float x;
    float y;
    float largura;
    float altura;

    float velocidadeY;

    bool atacando;
    int tempoAtaque;

    Vida vida;

} Jogador;

void DesenharHUD(Jogador jogador);
void ReceberDano(Jogador *jogador, int dano);

#endif
