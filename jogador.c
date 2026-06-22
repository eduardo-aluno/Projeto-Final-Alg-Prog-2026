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
    // 2. Desenha a Barra de Energia (Logo abaixo da vida)
    DrawRectangle(20, 60, 200, 20, DARKGRAY); // Fundo da barra
    // Calcula o tamanho da barra azul com base na energia atual (regra de três)
    DrawRectangle(20, 60, (jogador.energia * 200) / jogador.energiaMaxima, 20, BLUE);
    DrawText("ENERGIA", 25, 63, 15, WHITE);

    // 3. Desenha o Contador de Moedas (Logo abaixo da energia)
    DrawText(TextFormat("Moedas: %d", jogador.moedas), 20, 90, 20, GOLD);
}

void ReceberDano(Jogador *jogador, int dano)
{
    jogador->vida.atual -= dano;

    if (jogador->vida.atual < 0)
    {
        jogador->vida.atual = 0;
    }
}
