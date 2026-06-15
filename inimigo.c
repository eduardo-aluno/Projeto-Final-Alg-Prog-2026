#include "inimigo.h"

// Inicialize os inimigos em posições relativas ao mundo do mapa
void InicializarInimigos(Inimigo inimigos[], int quantidade, Texture2D texturaBase) {
    for (int i = 0; i < quantidade; i++) {
        inimigos[i].largura = 50;
        inimigos[i].altura = 50;
        // Posição inicial: vamos colocar eles longe do ponto de início do jogador
        inimigos[i].x = 600 + (i * 200);
        inimigos[i].y = 500; // Ajuste conforme o seu txt de mapa
        inimigos[i].velocidadeX = 2.0f;
        inimigos[i].vida = 3;
        inimigos[i].ativo = true;
        inimigos[i].sprite = texturaBase;
    }
}

// O restante das funções AtualizarInimigos e DesenharInimigos permanecem iguais
void AtualizarInimigos(Inimigo inimigos[], int quantidade, float limiteEsquerda, float limiteDireita) {
    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue; // Pula os que já morreram

        // Movimentação simples (patrulha)
        inimigos[i].x += inimigos[i].velocidadeX;

        // Inverte a direção se bater nos limites da janela
        if (inimigos[i].x <= limiteEsquerda || inimigos[i].x + inimigos[i].largura >= limiteDireita) {
            inimigos[i].velocidadeX *= -1;
        }
    }
}

void DesenharInimigos(Inimigo inimigos[], int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue;

        // Define a área da imagem original para renderizar
        Rectangle sourceRec = { 0.0f, 0.0f, (float)inimigos[i].sprite.width, (float)inimigos[i].sprite.height };

        // Define onde será desenhado na tela com o tamanho fixo (50x50)
        Rectangle destRec = { (float)inimigos[i].x, (float)inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

        // Desenha usando DrawTexturePro para forçar o redimensionamento da imagem
        DrawTexturePro(inimigos[i].sprite, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
    }
}
