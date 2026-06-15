#ifndef INIMIGO_H
#define INIMIGO_H

#include "raylib.h"

// Define a quantidade máxima de inimigos permitida
#define MAX_INIMIGOS 10

typedef struct {
    float x;
    float y;
    float largura;
    float altura;
    float velocidadeX;
    int vida;
    bool ativo;        // Para saber se o inimigo ainda está vivo
    Texture2D sprite;  // Armazena a imagem do inimigo
} Inimigo;

// Escopo das funções
void InicializarInimigos(Inimigo inimigos[], int quantidade, Texture2D texturaBase);
void AtualizarInimigos(Inimigo inimigos[], int quantidade, float limiteEsquerda, float limiteDireita);
void DesenharInimigos(Inimigo inimigos[], int quantidade);

#endif
