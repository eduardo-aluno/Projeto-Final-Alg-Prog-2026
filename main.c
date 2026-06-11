#include "raylib.h"
#define MENU 0
#define JOGO 1
#define PAUSE 2
#include "jogador.h"
#define GAMEOVER 3


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

    float chao = 700;

    jogador.x = 100;
    jogador.y = 600;
    jogador.largura = 50;
    jogador.altura = 80;

    jogador.atacando = false;
    jogador.tempoAtaque = 0;


    while (!WindowShouldClose())
    {

        if (estado == JOGO) // aqui vai toda a parte da fisica e da movimentacao
        {
            if (IsKeyDown(KEY_RIGHT))
            {
                jogador.x += 5;
            }

            if (IsKeyDown(KEY_LEFT))
            {
                jogador.x -= 5;
            }

            if (jogador.x < 0)
            {
                jogador.x = 0;
            }

            if (jogador.x > 1200 - jogador.largura)
            {
                jogador.x = 1200 - jogador.largura;
            }

            jogador.velocidadeY += 0.5;

            jogador.y += jogador.velocidadeY;

            if (jogador.y + jogador.altura >= chao)
            {
                jogador.y = chao - jogador.altura;
                jogador.velocidadeY = 0;
            }

            if (IsKeyPressed(KEY_UP))
            {
                if (jogador.y + jogador.altura >= chao)
                {
                    jogador.velocidadeY = -12;
                }
            }

            if (IsKeyPressed(KEY_S))
            {
                jogador.atacando = true;
                jogador.tempoAtaque = 15;
            }

            if (jogador.atacando)
            {
                jogador.tempoAtaque--;

                if (jogador.tempoAtaque <= 0)
                {
                    jogador.atacando = false;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE))
            {
                estado = PAUSE;

            }

            if (IsKeyPressed(KEY_K))
            {
                ReceberDano(&jogador, 1);
            }

            if (jogador.vida.atual <= 0)
            {
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
            DesenharHUD(jogador);

            DrawRectangle(
                0,
                chao,
                1200,
                100,
                DARKGRAY
            );

            DrawRectangle(
                jogador.x,
                jogador.y,
                jogador.largura,
                jogador.altura,
                WHITE
            );


            // Formato do DrawText -> (texto, x, y, tamanho, cor)
            DrawText(
                "Press S to attack",
                350,
                760,
                20,
                RED
            );


            if (jogador.atacando)
            {

                DrawRectangle(
                    jogador.x + jogador.largura,
                    jogador.y + 20, 40, 10,
                    YELLOW
                );

            }


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


