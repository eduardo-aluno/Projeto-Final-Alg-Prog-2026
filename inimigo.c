#include "inimigo.h"
#include "jogador.h" // Incluído para permitir a colisão e aplicação de dano direto

///////novas atualizacoes//////

// Estrutura simples para o projétil do chefe
typedef struct {
    float x;
    float y;
    float velocidadeX;
    float largura;
    float altura;
    bool ativo;
} ProjetilBoss;

// Instância global do projétil do chefe
ProjetilBoss projetilChefe = { 0, 0, 0, 15, 15, false };

// Função para verificar se um retângulo colide com blocos sólidos do mapa
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

// Função para movimentar o inimigo com colisão
void MoverInimigoComColisao(Inimigo *inimigo, float deltaX, float deltaY, Mapa mapa) {
    // Movimento em X
    inimigo->x += deltaX;
    Rectangle rectInimigo = {
        inimigo->x,
        inimigo->y,
        inimigo->largura,
        inimigo->altura
    };

    if (ColidindoComParedes(rectInimigo, mapa)) {
        // Se colidiu, desfaz o movimento em X
        inimigo->x -= deltaX;
        // Inverte a direção
        inimigo->velocidadeX *= -1;
    }

    // Movimento em Y
    inimigo->y += deltaY;
    rectInimigo.x = inimigo->x;
    rectInimigo.y = inimigo->y;

    if (ColidindoComParedes(rectInimigo, mapa)) {
        // Se colidiu, desfaz o movimento em Y
        inimigo->y -= deltaY;
    }
}

/////// fim das novas atualizacoes /////

// Inicialize os inimigos em posições relativas ao mundo do mapa
void InicializarInimigos(Inimigo inimigos[], int quantidade, Texture2D texturaBase) {
    for (int i = 0; i < quantidade; i++) {
        inimigos[i].largura = 50;
        inimigos[i].altura = 50;
        inimigos[i].x = 600 + (i * 200);
        inimigos[i].y = 500;
        inimigos[i].velocidadeX = 2.0f;
        inimigos[i].vida = 3;
        inimigos[i].ativo = true;
        inimigos[i].sprite = texturaBase;
        inimigos[i].eChefe = false;
    }
    // Garante que o projétil comece desativado ao iniciar a fase
    projetilChefe.ativo = false;
}

void InicializarBoss(Inimigo *boss, float startX, float startY, Texture2D texturaBase) {
    boss->largura = 80; // Boss um pouco maior
    boss->altura = 80;
    boss->x = startX;
    boss->y = startY;
    boss->velocidadeX = 1.5f;
    boss->vida = 10; // 3x mais vida que o normal
    boss->ativo = true;
    boss->sprite = texturaBase;
    boss->eChefe = true;
    boss->tempoAtaqueEspecial = 0;

    projetilChefe.ativo = false;
}

// ATUALIZADO: Agora aceita o ponteiro do jogador para aplicar o dano do projétil diretamente
void AtualizarInimigos(Inimigo inimigos[], int quantidade, float limiteEsquerda, float limiteDireita, Jogador *jogador, Mapa mapa) {
    float jogadorX = jogador->x;

    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue;

        if (inimigos[i].eChefe) {
            // --- INTELIGÊNCIA DO BOSS ---
            float deltaX = 0;
            if (jogadorX > inimigos[i].x) {
                deltaX = inimigos[i].velocidadeX;
            } else if (jogadorX < inimigos[i].x) {
                deltaX = -inimigos[i].velocidadeX;
            }

            // Movimento com colisão
            MoverInimigoComColisao(&inimigos[i], deltaX, 0, mapa);

            // --- LÓGICA DO ATAQUE ESPECIAL (PROJÉTIL) ---
            inimigos[i].tempoAtaqueEspecial++;

            // Dispara a cada 180 frames (aprox. 3 segundos em 60 FPS)
            if (inimigos[i].tempoAtaqueEspecial >= 180) {
                if (!projetilChefe.ativo) {
                    projetilChefe.ativo = true;
                    // Nasce no centro do Boss
                    projetilChefe.x = inimigos[i].x + inimigos[i].largura / 2;
                    projetilChefe.y = inimigos[i].y + inimigos[i].altura / 2 - 5;

                    // Define a direção baseada na posição do jogador
                    if (jogadorX > inimigos[i].x) {
                        projetilChefe.velocidadeX = 6.0f; // Dispara para a direita
                    } else {
                        projetilChefe.velocidadeX = -6.0f; // Dispara para a esquerda
                    }
                }
                inimigos[i].tempoAtaqueEspecial = 0; // Reseta o timer
            }

        } else {
            // --- INIMIGO NORMAL ---
            float deltaX = inimigos[i].velocidadeX;
            MoverInimigoComColisao(&inimigos[i], deltaX, 0, mapa);

            if (inimigos[i].x <= limiteEsquerda ||
                inimigos[i].x + inimigos[i].largura >= limiteDireita) {
                inimigos[i].velocidadeX *= -1;
            }
        }
    }

    // --- ATUALIZAÇÃO DO PROJÉTIL DO CHEFE ---
    if (projetilChefe.ativo) {
        projetilChefe.x += projetilChefe.velocidadeX;

        Rectangle rectProjetil = { projetilChefe.x, projetilChefe.y, projetilChefe.largura, projetilChefe.altura };
        Rectangle rectJogador = { jogador->x, jogador->y, jogador->largura, jogador->altura };

        // 1. Colisão com paredes desaparece o projétil
        if (ColidindoComParedes(rectProjetil, mapa)) {
            projetilChefe.ativo = false;
        }

        // 2. Colisão com o jogador causa dano (1 de vida) usando a sua função ReceberDano
        if (projetilChefe.ativo && CheckCollisionRecs(rectProjetil, rectJogador)) {
            ReceberDano(jogador, 1);
            projetilChefe.ativo = false; // Projétil some ao atingir
        }
    }
}

void DesenharInimigos(Inimigo inimigos[], int quantidade) {
    for (int i = 0; i < quantidade; i++) {
        if (!inimigos[i].ativo) continue;

        Rectangle sourceRec = { 0.0f, 0.0f, (float)inimigos[i].sprite.width, (float)inimigos[i].sprite.height };
        Rectangle destRec = { (float)inimigos[i].x, (float)inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

        DrawTexturePro(inimigos[i].sprite, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // --- DESENHAR O PROJÉTIL (Caso esteja ativo) ---
    if (projetilChefe.ativo) {
        // Desenha uma esfera de energia laranja/vermelha simulando um ataque mágico
        DrawCircle((int)projetilChefe.x + projetilChefe.largura/2, (int)projetilChefe.y + projetilChefe.altura/2, projetilChefe.largura, ORANGE);
        DrawCircle((int)projetilChefe.x + projetilChefe.largura/2, (int)projetilChefe.y + projetilChefe.altura/2, projetilChefe.largura - 4, YELLOW);
    }
}
