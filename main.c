#include "raylib.h"

#define MENU 0
#define JOGO 1
#define PAUSE 2
#define GAMEOVER 3

#include "jogador.h"
#include "mapa.h"



int main()
{


    InitWindow(1200, 800, "Hollow Knight");
    SetExitKey(KEY_NULL);
    Texture2D imagemPause = LoadTexture("assets/image.pause.png");

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

   Mapa mapaAtual = CarregarMapa("D:/Personal/Faculdade/Semestres/CIC/2026-1/cadeiras/alg prog/JOGO/HollowKni/bin/Debug/o_teste.txt");


    InicializarPosicaoJogador(mapaAtual, &jogador);


    Camera2D camera = { 0 };
    camera.target = (Vector2){ jogador.x, jogador.y };
    camera.offset = (Vector2){ 1200 / 2.0f, 800 / 2.0f }; // Centraliza o Knight na janela
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;


    while (!WindowShouldClose())
    {

        if (estado == JOGO) // Física, movimentação e colisões completas
        {
           // =====================================================================
            // 1. MOVIMENTAÇÃO E COLISÃO HORIZONTAL (EIXO X)
            // =====================================================================
            float deltaX = 0;
            if (IsKeyDown(KEY_RIGHT)) {
                deltaX = 5;
            }
            if (IsKeyDown(KEY_LEFT)) {
                deltaX = -5;
            }

            jogador.x += deltaX;

            Rectangle hitboxJogador = { jogador.x, jogador.y, jogador.largura, jogador.altura };

            int colunaInicio = (int)(jogador.x / 50) - 2;
            int colunaFim    = (int)((jogador.x + jogador.largura) / 50) + 2;
            int linhaInicio  = (int)(jogador.y / 50) - 2;
            int linhaFim     = (int)((jogador.y + jogador.altura) / 50) + 2;

            if (colunaInicio < 0) colunaInicio = 0;
            if (colunaFim > MAPA_COLUNAS) colunaFim = MAPA_COLUNAS;
            if (linhaInicio < 0) linhaInicio = 0;
            if (linhaFim > MAPA_LINHAS) linhaFim = MAPA_LINHAS;

            for (int l = linhaInicio; l < linhaFim; l++)
            {
                for (int c = colunaInicio; c < colunaFim; c++)
                {
                    if (mapaAtual.matriz[l][c] == 'P' || mapaAtual.matriz[l][c] == 'p')
                    {
                        Rectangle hitboxBloco = { c * 50, l * 50, 50, 50 };

                        if (CheckCollisionRecs(hitboxJogador, hitboxBloco))
                        {
                            if (deltaX > 0)
                            {
                                jogador.x = hitboxBloco.x - jogador.largura;
                            }
                            else if (deltaX < 0)
                            {
                                jogador.x = hitboxBloco.x + hitboxBloco.width;
                            }
                            hitboxJogador.x = jogador.x;
                        }
                    }
                }
            }

            if (jogador.x < 0) jogador.x = 0;
            if (jogador.x > (MAPA_COLUNAS * TAMANHO_BLOCO) - jogador.largura)
                jogador.x = (MAPA_COLUNAS * TAMANHO_BLOCO) - jogador.largura;


            // =====================================================================
            // 2. APLICAÇÃO DA GRAVIDADE E COLISÃO VERTICAL (EIXO Y)
            // =====================================================================
            jogador.velocidadeY += 0.5;
            jogador.y += jogador.velocidadeY;

            hitboxJogador.y = jogador.y;
            hitboxJogador.x = jogador.x;

            for (int l = linhaInicio; l < linhaFim; l++)
            {
                for (int c = colunaInicio; c < colunaFim; c++)
                {
                    if (mapaAtual.matriz[l][c] == 'P' || mapaAtual.matriz[l][c] == 'p')
                    {
                        Rectangle hitboxBloco = { c * 50, l * 50, 50, 50 };

                        if (CheckCollisionRecs(hitboxJogador, hitboxBloco))
                        {
                            if (jogador.velocidadeY > 0)
                            {
                                jogador.y = hitboxBloco.y - jogador.altura;
                                jogador.velocidadeY = 0;
                            }
                            else if (jogador.velocidadeY < 0)
                            {
                                jogador.y = hitboxBloco.y + hitboxBloco.height;
                                jogador.velocidadeY = 0;
                            }
                            hitboxJogador.y = jogador.y;
                        }
                    }
                }
            }
            // =====================================================================

            // 3. SISTEMA DE PULO (Permite pular apenas se estiver firmemente no chão)
            if (IsKeyPressed(KEY_UP))
            {
                if (jogador.velocidadeY == 0)
                {
                    jogador.velocidadeY = -12; // Altura do pulo
                }
            }

             // Limitadores das bordas externas do mapa (150 colunas * 50 pixels = 7500 pixels de largura)
            if (jogador.x < 0) jogador.x = 0;
            if (jogador.x > (MAPA_COLUNAS * TAMANHO_BLOCO) - jogador.largura)
                jogador.x = (MAPA_COLUNAS * TAMANHO_BLOCO) - jogador.largura;

            // 4. ATUALIZAÇÃO DA CÂMERA SEGUIDORA
            camera.target.x = jogador.x + jogador.largura / 2.0f;
            camera.target.y = 400; // Mantém a visão vertical centralizada e estável

            // Lógica de ataque do Knight
            if (IsKeyPressed(KEY_S)) {
                jogador.atacando = true;
                jogador.tempoAtaque = 15;
            }
            if (jogador.atacando) {
                jogador.tempoAtaque--;
                if (jogador.tempoAtaque <= 0) jogador.atacando = false;
            }

            // Lógicas de Pausa, Dano e Fim de Jogo
            if (IsKeyPressed(KEY_ESCAPE)) {
                estado = PAUSE;
            }
            if (IsKeyPressed(KEY_K)) {
                ReceberDano(&jogador, 1);
            }
            if (jogador.vida.atual <= 0) {
                estado = GAMEOVER;
            }

        } // FECHA O if (estado == JOGO)

        BeginDrawing();

        ClearBackground(BLACK);

        if (estado == MENU)
        {
            DrawText("HOLLOW KNIGHT", 450, 200, 40, WHITE);
            DrawText("J - Jogar", 500, 350, 30, WHITE);
            DrawText("S - Sair", 500, 400, 30, WHITE);

            if (IsKeyPressed(KEY_J))
            {
                estado = JOGO;
            }

            if (IsKeyPressed(KEY_S))
            {
                break;
            }
        }

        else if (estado == JOGO)
        {


            BeginMode2D(camera);

            DesenharMapa(mapaAtual); // Desenha as paredes e estruturas de fundo

            // Desenha o Knight
            DrawRectangle(jogador.x, jogador.y, jogador.largura, jogador.altura, WHITE);

            // Desenha o ataque do Knight
            if (jogador.atacando) {
                DrawRectangle(jogador.x + jogador.largura, jogador.y + 20, 40, 10, YELLOW);
            }

        EndMode2D();


              DesenharHUD(jogador);


            // Formato do DrawText -> (texto, x, y, tamanho, cor)
            DrawText(
                "Press S to attack",
                350,
                760,
                20,
                RED
            );




            DrawText(
                "Press ESC to Pause",
                20,
                760,
                20,
                WHITE
            );


        }
        else if (estado == PAUSE)
        {
            DrawTexture(
                imagemPause,
                0,
                0,
                WHITE
            );

            DrawText(
                "GAME PAUSED",
                430,
                300,
                40,
                MAROON
            );

            DrawText(
                "Press C to Continue",
                380,
                400,
                30,
                MAROON
            );

            DrawText(
                "Press S to Exit to Menu",
                350,
                450,
                30,
                MAROON
            );

            if (IsKeyPressed(KEY_C))
            {
                estado = JOGO;
            }

            if (IsKeyPressed(KEY_S))
            {
                estado = MENU;
            }
        }

        else if (estado == GAMEOVER)
        {
            DrawText(
                "GAME OVER",
                450,
                300,
                50,
                RED
            );

            DrawText(
                "Press M to return to Menu",
                350,
                400,
                30,
                WHITE
            );


    if (IsKeyPressed(KEY_M))
    {
        jogador.vida.atual = jogador.vida.maxima;
        estado = MENU;
    }
        }

            EndDrawing();

    }


        CloseWindow();
        return 0;

    }


