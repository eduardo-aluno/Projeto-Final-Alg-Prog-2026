#include "jogador.h"

void DesenharHUD(Jogador jogador)
{
    for (int i = 0; i < jogador.vida.maxima; i++)
    {
        Color cor = DARKGRAY;

        if (i < jogador.vida.atual)
        {
            cor = RED;
        }

        DrawRectangle(
            20 + i * 35,
            20,
            25,
            25,
            cor
        );
    }
}

void ReceberDano(Jogador *jogador, int dano)
{
    jogador->vida.atual -= dano;

    if (jogador->vida.atual < 0)
    {
        jogador->vida.atual = 0;
    }
}
