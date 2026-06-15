#include "raylib.h"
#include "inimigo.h"
#include "jogador.h"
#include "mapa.h"

#define MENU 0
#define JOGO 1
#define PAUSE 2
#define GAMEOVER 3
#define CREDITOS 4

int main()
{
    InitWindow(800, 600, "Hollow Knight");
    SetExitKey(KEY_NULL);

    Texture2D imagemPause = LoadTexture("assets/image.pause.png");
    Texture2D spriteInimigo = LoadTexture("assets/inimigo.png");
    Texture2D spriteJogador = LoadTexture("assets/jogador.png");

    SetTargetFPS(60);

    Jogador jogador;
    jogador.vida.atual = 5;
    jogador.vida.maxima = 5;
    int estado = MENU;
    jogador.velocidadeY = 0;
    jogador.largura = 50;
    jogador.altura = 50;
    jogador.atacando = false;
    jogador.tempoAtaque = 0;
    jogador.tempoInvencivel = 0;
    jogador.olhandoDireita = true;
    jogador.sprite = spriteJogador;

    Mapa mapaAtual = CarregarMapa("bin/Debug/o_teste.txt");
    InicializarPosicaoJogador(mapaAtual, &jogador);

    Inimigo inimigos[MAX_INIMIGOS];
    InicializarInimigos(inimigos, 3, spriteInimigo);

    Camera2D camera = { 0 };
    camera.target = (Vector2)
    {
        jogador.x, jogador.y
    };
    camera.offset = (Vector2)
    {
        400.0f, 300.0f
    };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Loop principal
    while (!WindowShouldClose())
    {
        // ==========================================
        // 1. LÓGICA DO JOGO (Física, Input, Estados)
        // ==========================================
        if (estado == JOGO)
        {
            // Diminuir o tempo de invencibilidade do jogador (se houver)
            if (jogador.tempoInvencivel > 0) jogador.tempoInvencivel--;

            // --- MOVIMENTAÇÃO NO EIXO X ---
            float deltaX = 0;
            if (IsKeyDown(KEY_RIGHT))
            {
                deltaX = 5;
                jogador.olhandoDireita = true;
            }
            if (IsKeyDown(KEY_LEFT))
            {
                deltaX = -5;
                jogador.olhandoDireita = false;
            }

            jogador.x += deltaX;

            Rectangle rectJogador = { jogador.x, jogador.y, jogador.largura, jogador.altura };

            // Varre o mapa para colisão em X (Paredes)
            for (int i = 0; i < MAPA_LINHAS; i++) // Substitua por MAPA_LINHAS ou o nome da sua constante de linhas
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'P')
                    {
                        Rectangle rectBloco = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };
                        if (CheckCollisionRecs(rectJogador, rectBloco))
                        {
                            if (deltaX > 0) jogador.x = rectBloco.x - jogador.largura;
                            else if (deltaX < 0) jogador.x = rectBloco.x + TAMANHO_BLOCO;
                            rectJogador.x = jogador.x;
                        }
                    }
                }
            }

            // --- MOVIMENTAÇÃO NO EIXO Y (Gravidade e Pulo) ---
            jogador.velocidadeY += 0.5f;
            jogador.y += jogador.velocidadeY;
            rectJogador.y = jogador.y;

            bool noChao = false;

            // Varre o mapa para colisão em Y (Chão e Teto)
            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'P')
                    {
                        Rectangle rectBloco = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };
                        if (CheckCollisionRecs(rectJogador, rectBloco))
                        {
                            if (jogador.velocidadeY > 0) // Caindo
                            {
                                jogador.y = rectBloco.y - jogador.altura;
                                jogador.velocidadeY = 0;
                                noChao = true;
                            }
                            else if (jogador.velocidadeY < 0) // Subindo (Bater a cabeça)
                            {
                                jogador.y = rectBloco.y + TAMANHO_BLOCO;
                                jogador.velocidadeY = 0;
                            }
                            rectJogador.y = jogador.y;
                        }
                    }
                }
            }

            camera.target = (Vector2)
            {
                jogador.x, jogador.y
            };
            AtualizarInimigos(inimigos, 3, 0, MAPA_COLUNAS * TAMANHO_BLOCO);

            if (IsKeyPressed(KEY_UP) && noChao)
            {
                jogador.velocidadeY = -12;
            }

            // --- CONTROLE DE ATAQUE DO JOGADOR ---
            if (IsKeyPressed(KEY_S) && !jogador.atacando)
            {
                jogador.atacando = true;
                jogador.tempoAtaque = 15;

                Rectangle rectAtaque;
                if (jogador.olhandoDireita)
                {
                    rectAtaque = (Rectangle)
                    {
                        jogador.x + jogador.largura, jogador.y + 10, 40, 30
                    };
                }
                else
                {
                    rectAtaque = (Rectangle)
                    {
                        jogador.x - 40, jogador.y + 10, 40, 30
                    };
                }

                for (int i = 0; i < 3; i++)
                {
                    if (inimigos[i].ativo)
                    {
                        Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };
                        if (CheckCollisionRecs(rectAtaque, rectInimigo))
                        {
                            inimigos[i].vida -= 1;
                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;
                            }
                        }
                    }
                }
            }

            // Duração do Ataque
            if (jogador.atacando)
            {
                jogador.tempoAtaque--;
                if (jogador.tempoAtaque <= 0) jogador.atacando = false;
            }

            // --- SISTEMA DE COMBATE: INIMIGO DANDO DANO NO JOGADOR ---
            for (int i = 0; i < 3; i++)
            {
                if (inimigos[i].ativo)
                {
                    Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

                    if (CheckCollisionRecs(rectJogador, rectInimigo) && jogador.tempoInvencivel == 0)
                    {
                        ReceberDano(&jogador, 1);
                        jogador.tempoInvencivel = 60; // 1 segundo de imunidade

                        if (jogador.vida.atual <= 0)
                        {
                            estado = GAMEOVER;
                        }
                    }
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                estado = PAUSE;
            }
        }
        else if (estado == PAUSE)
        {
            if (IsKeyPressed(KEY_C)) estado = JOGO;
        }

        // ==========================================
        // 2. DESENHO DO JOGO (Renderização na tela)
        // ==========================================
        BeginDrawing();
        ClearBackground(BLACK);

        if (estado == MENU)
        {
            DrawText("HOLLOW KNIGHT", 250, 200, 40, WHITE);
            DrawText("J - Jogar", 350, 350, 30, WHITE);
            DrawText("S - Sair", 350, 400, 30, WHITE);
            DrawText("C - CREDITOS", 290, 450, 30, WHITE);

            if (IsKeyPressed(KEY_J))
            {
                estado = JOGO;
            }
            if (IsKeyPressed(KEY_S))
            {
                CloseWindow();
            }
            if (IsKeyPressed(KEY_C))
            {
                estado = CREDITOS;
            }
        }
        else if (estado == JOGO)
        {
            BeginMode2D(camera);
            DesenharMapa(mapaAtual);
            DesenharInimigos(inimigos, 3);

            Rectangle sourceRec = { 0.0f, 0.0f, (float)jogador.sprite.width, (float)jogador.sprite.height };
            Rectangle destRec = { jogador.x, jogador.y, (float)jogador.largura, (float)jogador.altura };
            Vector2 origin = { 0.0f, 0.0f };

            // Faz o jogador piscar quando estiver invencível
            if (jogador.tempoInvencivel == 0 || (jogador.tempoInvencivel / 5) % 2 == 0)
            {
                DrawTexturePro(jogador.sprite, sourceRec, destRec, origin, 0.0f, WHITE);
            }

            // O ataque desenhado dependendo do lado para onde olha
            if (jogador.atacando)
            {
                if (jogador.olhandoDireita)
                {
                    DrawRectangle(jogador.x + jogador.largura, jogador.y + 10, 40, 30, YELLOW);
                }
                else
                {
                    DrawRectangle(jogador.x - 40, jogador.y + 10, 40, 30, YELLOW);
                }
            }
            EndMode2D();

            DesenharHUD(jogador);

            DrawText("Press S to attack", 350, 550, 20, RED);
            DrawText("Press ESC to Pause", 20, 550, 20, WHITE);
        }
        else if (estado == PAUSE)
        {
            DrawTexture(imagemPause, 0, 0, WHITE);

            DrawText("GAME PAUSED", 230, 200, 40, MAROON);
            DrawText("Press C to Continue", 180, 300, 30, MAROON);
            DrawText("Press S to Exit to Menu", 150, 350, 30, MAROON);

            if (IsKeyPressed(KEY_C))
                estado = JOGO;

            if (IsKeyPressed(KEY_S))
                estado = MENU;
        }

        else if (estado == CREDITOS)
        {

            DrawText("CREDITOS", 280, 40, 40, WHITE);

            DrawText("Projeto Final de Algoritmos e Programacao", 80, 120, 25, WHITE);

            DrawText("Desenvolvido por:", 80, 180, 25, YELLOW);

            DrawText("Eduardo dos Santos Barbosa", 120, 230, 20, WHITE);
            DrawText("Mateo Alfonso Castiglia Pacheco", 120, 260, 20, WHITE);

            DrawText("Universidade Federal do Rio Grande do Sul", 80, 340, 20, LIGHTGRAY);

            DrawText("Pressione ESC para voltar", 180, 520, 20, GRAY);

            if (IsKeyPressed(KEY_ESCAPE))
                estado = MENU;
        }

        else if (estado == GAMEOVER)
        {
            DrawText("GAME OVER", 250, 250, 50, RED);
            DrawText("Press M to return to Menu", 150, 350, 30, WHITE);

            if (IsKeyPressed(KEY_M))
            {
                jogador.vida.atual = jogador.vida.maxima;
                // Reseta a posição do jogador para não dar Game Over infinito
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = MENU;
            }
        }



        EndDrawing();
    }

    UnloadTexture(spriteInimigo);
    UnloadTexture(spriteJogador);
    UnloadTexture(imagemPause);
    CloseWindow();

    return 0;
}
