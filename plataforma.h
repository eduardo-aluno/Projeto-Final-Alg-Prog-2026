#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#include "raylib.h"
#include "mapa.h"
#include "jogador.h"

#define MAX_PLATAFORMAS 10

typedef struct {
    float x;
    float y;
    float largura;
    float altura;
    float velocidadeX;
    float limiteEsquerda;
    float limiteDireita;
    bool ativo;
} PlataformaMovel;

// Protótipos das funções para que outros ficheiros as consigam ver
void SpawnarPlataformas(Mapa mapa, PlataformaMovel plataformas[]);
void AtualizarPlataformas(PlataformaMovel plataformas[], Jogador *jogador, bool *noChao);
void DesenharPlataformas(PlataformaMovel plataformas[], Texture2D textura);

#endif // PLATAFORMA_H
