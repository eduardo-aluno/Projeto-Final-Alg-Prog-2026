#include "inimigo.h"
#include "jogador.h" // Incluído para permitir a colisão e aplicação de dano/energia direto
#include <math.h>

// Estrutura interna para os projéteis do chefe
typedef struct {
    float x;
    float y;
    float velocidadeX;
    float largura;
    float altura;
    bool ativo;
} ProjetilBoss;

// Instâncias globais dos dois projéteis do chefe
ProjetilBoss projetilChefe = { 0, 0, 0, 15, 15, false };       // Bolinha Amarela (Tira Vida)
ProjetilBoss projetilVermelho = { 0, 0, 0, 15, 15, false };    // Bolinha Vermelha (Tira 10% de Energia)

// Função auxiliar para verificar se um retângulo colide com blocos sólidos do mapa
bool ColidindoComParedes(Rectangle rect, Mapa mapa) {
    for (int i = 0; i < MAPA_LINHAS; i++) {
        for (int j = 0; j < MAPA_COLUNAS; j++) {
            if (mapa.matriz[i][j] == 'P') {
                Rectangle rectBloco = {
                    j * TAMANHO_BLOCO,
                    i * TAMANHO_BLOCO,
                    TAMANHO_BLOCO,
                    TAMANHO_BLOCO
                };
                if (CheckCollisionRecs(rect, rectBloco)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Função para inicializar os monstros comuns do mapa
void InicializarInimigos(Inimigo inimigos[], int quantidade, Texture2D texturaBase) {
    for (int i = 0; i < quantidade; i++) {
        inimigos[i].largura = 50;
        inimigos[i].altura = 50;
        inimigos[i].velocidadeX = 2.0f;
        inimigos[i].vida = 2;
        inimigos[i].ativo = false;
        inimigos[i].sprite = texturaBase;
        inimigos[i].eChefe = false;
        inimigos[i].tempoAtaqueEspecial = 0;
    }
}

// Função para inicializar as propriedades exclusivas do Boss
void InicializarBoss(Inimigo *boss, float startX, float startY, Texture2D texturaBase) {
    boss->x = startX;
    boss->y = startY;
    boss->largura = 80;  // Boss é ligeiramente maior
    boss->altura = 80;
    boss->velocidadeX = 1.5f;
    boss->vida = 10;     // Mais vida que monstros normais
    boss->ativo = true;
    boss->sprite = texturaBase;
    boss->eChefe = true;
    boss->tempoAtaqueEspecial = 0;
}

// Atualização de IA, movimentações, ataques e colisões de projéteis
void AtualizarInimigos(Inimigo inimigos[], int quantidade, float limiteEsquerda, float limiteDireita, Jogador *jogador, Mapa mapa) {
    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue;

        if (inimigos[i].eChefe) {
            // --- MOVIMENTAÇÃO DO BOSS (Persegue a posição X do jogador lentamente) ---
            if (jogador->x < inimigos[i].x) {
                inimigos[i].x -= 1.0f;
            } else {
                inimigos[i].x += 1.0f;
            }

            // --- INTELIGÊNCIA DE ATAQUE DO BOSS ---
            inimigos[i].tempoAtaqueEspecial++;

            // A cada 120 frames (~2 segundos a 60 FPS), o Boss decide o ataque
            if (inimigos[i].tempoAtaqueEspecial >= 120) {
                inimigos[i].tempoAtaqueEspecial = 0;

                // Sorteia o ataque: 0 para Amarelo (Vida), 1 para Vermelho (Energia)
                int tipoAtaque = GetRandomValue(0, 1);

                if (tipoAtaque == 0 && !projetilChefe.ativo) {
                    // Dispara Bolinha Amarela (Vida)
                    projetilChefe.x = inimigos[i].x + (inimigos[i].largura / 2);
                    projetilChefe.y = inimigos[i].y + 20;
                    projetilChefe.velocidadeX = (jogador->x < inimigos[i].x) ? -6.0f : 6.0f;
                    projetilChefe.ativo = true;
                }
                else if (tipoAtaque == 1 && !projetilVermelho.ativo) {
                    // Dispara Bolinha Vermelha (Energia)
                    projetilVermelho.x = inimigos[i].x + (inimigos[i].largura / 2);
                    projetilVermelho.y = inimigos[i].y + 20;
                    projetilVermelho.velocidadeX = (jogador->x < inimigos[i].x) ? -6.0f : 6.0f;
                    projetilVermelho.ativo = true;
                }
            }
        } else {
            // --- MOVIMENTAÇÃO DO MONSTRO COMUM (Patrulha padrão com limites e paredes) ---
            inimigos[i].x += inimigos[i].velocidadeX;
            Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

            if (ColidindoComParedes(rectInimigo, mapa) || inimigos[i].x <= limiteEsquerda || inimigos[i].x >= limiteDireita) {
                inimigos[i].velocidadeX *= -1;
            }
        }
    }

    // --- FÍSICA E COLISÃO DO PROJÉTIL AMARELO (Dano de Vida) ---
    if (projetilChefe.ativo) {
        projetilChefe.x += projetilChefe.velocidadeX;
        Rectangle rectProjetil = { projetilChefe.x, projetilChefe.y, projetilChefe.largura, projetilChefe.altura };
        Rectangle rectJogador = { jogador->x, jogador->y, jogador->largura, jogador->altura };

        if (ColidindoComParedes(rectProjetil, mapa)) {
            projetilChefe.ativo = false;
        }

        if (projetilChefe.ativo && CheckCollisionRecs(rectProjetil, rectJogador)) {
            ReceberDano(jogador, 1); // Causa 1 de dano usando a mecânica interna
            projetilChefe.ativo = false;
        }
    }

    // --- FÍSICA E COLISÃO DO PROJÉTIL VERMELHO (Dano de 10% de Energia) ---
    if (projetilVermelho.ativo) {
        projetilVermelho.x += projetilVermelho.velocidadeX;
        Rectangle rectProjetilV = { projetilVermelho.x, projetilVermelho.y, projetilVermelho.largura, projetilVermelho.altura };
        Rectangle rectJogador = { jogador->x, jogador->y, jogador->largura, jogador->altura };

        if (ColidindoComParedes(rectProjetilV, mapa)) {
            projetilVermelho.ativo = false;
        }

        if (projetilVermelho.ativo && CheckCollisionRecs(rectProjetilV, rectJogador)) {
            // Reduz 10% do total máximo de energia (10% de 100 = 10 pontos)
            int perdaEnergia = (int)(jogador->energiaMaxima * 0.10f);
            jogador->energia -= perdaEnergia;

            if (jogador->energia < 0) {
                jogador->energia = 0; // Impede que a energia fique negativa
            }

            projetilVermelho.ativo = false;
        }
    }
}

// Renderização dos sprites dos inimigos e desenho dos círculos de ataque
void DesenharInimigos(Inimigo inimigos[], int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue;

        Rectangle sourceRec = { 0.0f, 0.0f, (float)inimigos[i].sprite.width, (float)inimigos[i].sprite.height };
        Rectangle destRec = { (float)inimigos[i].x, (float)inimigos[i].y, inimigos[i].largura, inimigos[i].altura };
        DrawTexturePro(inimigos[i].sprite, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // --- DESENHAR PROJÉTIL AMARELO (Caso esteja ativo) ---
    if (projetilChefe.ativo) {
        DrawCircle(projetilChefe.x + 7.5f, projetilChefe.y + 7.5f, 8, GOLD);
        DrawCircle(projetilChefe.x + 7.5f, projetilChefe.y + 7.5f, 5, YELLOW);
    }

    // --- DESENHAR PROJÉTIL VERMELHO (Caso esteja ativo) ---
    if (projetilVermelho.ativo) {
        DrawCircle(projetilVermelho.x + 7.5f, projetilVermelho.y + 7.5f, 8, MAROON);
        DrawCircle(projetilVermelho.x + 7.5f, projetilVermelho.y + 7.5f, 5, RED);
    }
}
