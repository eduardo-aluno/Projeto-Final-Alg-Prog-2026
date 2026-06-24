#ifndef MAPA_H
#define MAPA_H

#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define MAPA_LINHAS 16
#define MAPA_COLUNAS 150
#define TAMANHO_BLOCO 50 // Cada caractere vira um bloco de 50x50 pixels na tela



typedef struct {
    char matriz[MAPA_LINHAS][MAPA_COLUNAS];

} Mapa;

typedef struct Jogador Jogador;

// Funções para gerenciar o mapa
Mapa CarregarMapa(const char *caminhoArquivo);
void DesenharMapa(Mapa mapa);

void InicializarPosicaoJogador(Mapa mapa, Jogador *jogador);

void SetTexturaChao(Texture2D textura);

#endif
