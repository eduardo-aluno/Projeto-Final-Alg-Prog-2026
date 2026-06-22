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
// --- NOVO: Variáveis para o Chefe ---
    bool eChefe;
    int tempoAtaqueEspecial; // Para controlar quando ele usa o segundo ataque
} Inimigo;

// Escopo das funções
void InicializarInimigos(Inimigo inimigos[], int quantidade, Texture2D texturaBase);

// NOVO: Escopo do Boss
void InicializarBoss(Inimigo *boss, float startX, float startY, Texture2D texturaBase);

// ATUALIZADO: Agora com o float jogadorX no final
void AtualizarInimigos(Inimigo inimigos[], int quantidade, float limiteEsquerda, float limiteDireita, float jogadorX);

void DesenharInimigos(Inimigo inimigos[], int quantidade);
#endif
