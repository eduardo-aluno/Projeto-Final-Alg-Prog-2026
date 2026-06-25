#include "plataforma.h"

void SpawnarPlataformas(Mapa mapa, PlataformaMovel plataformas[])
{
    for (int i = 0; i < MAX_PLATAFORMAS; i++) {
        plataformas[i].ativo = false;
    }

    int index = 0;

    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            if (mapa.matriz[l][c] == 'S' && index < MAX_PLATAFORMAS) {
                plataformas[index].largura = TAMANHO_BLOCO;
                plataformas[index].altura = TAMANHO_BLOCO;
                plataformas[index].x = c * TAMANHO_BLOCO;
                plataformas[index].y = l * TAMANHO_BLOCO;

                plataformas[index].velocidadeX = 2.0f;
                plataformas[index].limiteEsquerda = plataformas[index].x - (TAMANHO_BLOCO * 3);
                plataformas[index].limiteDireita = plataformas[index].x + (TAMANHO_BLOCO * 3);
                plataformas[index].ativo = true;
                index++;

                // Remove o caractere para não colidir como bloco estático
                mapa.matriz[l][c] = ' ';
            }
        }
    }
}

void AtualizarPlataformas(PlataformaMovel plataformas[], Jogador *jogador, bool *noChao)
{
    for (int i = 0; i < MAX_PLATAFORMAS; i++)
    {
        if (plataformas[i].ativo)
        {
            // 1. Movimentação horizontal da plataforma
            plataformas[i].x += plataformas[i].velocidadeX;

            if (plataformas[i].x <= plataformas[i].limiteEsquerda || plataformas[i].x >= plataformas[i].limiteDireita) {
                plataformas[i].velocidadeX *= -1;
            }

            // 2. Física e Colisão com o Jogador (CORRIGIDO AQUI)
            Rectangle rectPlat = { plataformas[i].x, plataformas[i].y, plataformas[i].largura, plataformas[i].altura };
            Rectangle rectJogador = { jogador->x, jogador->y, jogador->largura, jogador->altura };

            // Verifica se as caixas de colisão se cruzam
            if (CheckCollisionRecs(rectJogador, rectPlat))
            {
                // Verifica se o jogador está caindo ou pisando no topo da plataforma (com tolerância de 10 pixels)
                if (jogador->velocidadeY >= 0 && (jogador->y + jogador->altura - jogador->velocidadeY) <= plataformas[i].y + 10)
                {
                    jogador->y = plataformas[i].y - jogador->altura;
                    jogador->velocidadeY = 0;
                    *noChao = true;

                    // Faz o jogador se mover junto com a plataforma horizontalmente
                    jogador->x += plataformas[i].velocidadeX;
                }
            }
        }
    }
}

void DesenharPlataformas(PlataformaMovel plataformas[], Texture2D textura)
{
    for (int i = 0; i < MAX_PLATAFORMAS; i++) {
        if (plataformas[i].ativo) {
            // CORREÇÃO: Redimensiona a imagem para caber perfeitamente no bloco de 50x50
            Rectangle sourceRec = { 0.0f, 0.0f, (float)textura.width, (float)textura.height };
            Rectangle destRec = { plataformas[i].x, plataformas[i].y, plataformas[i].largura, plataformas[i].altura };
            Vector2 origin = { 0.0f, 0.0f };
            DrawTexturePro(textura, sourceRec, destRec, origin, 0.0f, WHITE);
        }
    }
}
