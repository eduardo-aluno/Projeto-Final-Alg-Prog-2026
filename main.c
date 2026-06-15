#include "raylib.h"

#define MENU 0
#define JOGO 1
#define PAUSE 2
#define GAMEOVER 3
#include "inimigo.h"
#include "jogador.h"
#include "mapa.h"

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
    jogador.sprite = spriteJogador;

    Mapa mapaAtual = CarregarMapa("bin/Debug/o_teste.txt");
    InicializarPosicaoJogador(mapaAtual, &jogador);

    Inimigo inimigos[MAX_INIMIGOS];
    InicializarInimigos(inimigos, 3, spriteInimigo);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ jogador.x, jogador.y };
    camera.offset = (Vector2){ 400.0f, 300.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // O loop principal começa aqui e deve conter TANTO a lógica QUANTO o desenho
    while (!WindowShouldClose())
    {
        // ==========================================
        // 1. LÓGICA DO JOGO (Física, Input, Estados)
        // ==========================================
        if (estado == JOGO)
        {
            float deltaX = 0;
            if (IsKeyDown(KEY_RIGHT)) deltaX = 5;
            if (IsKeyDown(KEY_LEFT)) deltaX = -5;
            jogador.x += deltaX;

            jogador.velocidadeY += 0.5f;
            jogador.y += jogador.velocidadeY;

            // Colisão com o chão provisória
            if (jogador.y > 500) { jogador.y = 500; jogador.velocidadeY = 0; }
            if (IsKeyPressed(KEY_UP) && jogador.velocidadeY == 0) jogador.velocidadeY = -12;

            camera.target = (Vector2){ jogador.x, jogador.y };
            AtualizarInimigos(inimigos, 3, 0, MAPA_COLUNAS * TAMANHO_BLOCO);

            // Controle de Ataque e Pause
            if (IsKeyPressed(KEY_S)) {
                jogador.atacando = true;
                jogador.tempoAtaque = 15;
            }
            if (IsKeyPressed(KEY_ESCAPE)) {
                estado = PAUSE;
            }

            // Duração do Ataque
            if (jogador.atacando) {
                jogador.tempoAtaque--;
                if (jogador.tempoAtaque <= 0) jogador.atacando = false;
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

            if (IsKeyPressed(KEY_J))
            {
                estado = JOGO;
            }
            if (IsKeyPressed(KEY_S))
            {
                CloseWindow(); // Descomentado para que o S feche o jogo aqui
            }
        }
        else if (estado == JOGO)
        {
BeginMode2D(camera);
    DesenharMapa(mapaAtual);
    DesenharInimigos(inimigos, 3);

    // 1. Define a origem: a imagem inteira (do 0,0 até a largura/altura total dela)
    Rectangle sourceRec = { 0.0f, 0.0f, (float)jogador.sprite.width, (float)jogador.sprite.height };

    // 2. Define o destino: onde o jogador está no mundo e o tamanho desejado (50x50)
    Rectangle destRec = { jogador.x, jogador.y, (float)jogador.largura, (float)jogador.altura };

    // 3. Define o ponto de origem (pivot) para rotação/posicionamento (0,0 é o canto superior esquerdo)
    Vector2 origin = { 0.0f, 0.0f };

    // 4. Desenha usando o Pro
    DrawTexturePro(jogador.sprite, sourceRec, destRec, origin, 0.0f, WHITE);

    // O ataque desenhado no mundo (ainda como retângulo)
    if (jogador.atacando) {
        DrawRectangle(jogador.x + jogador.largura, jogador.y + 20, 40, 10, YELLOW);
    }
EndMode2D();

            // A HUD fica fora da câmera para acompanhar a tela
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
        else if (estado == GAMEOVER)
        {
            DrawText("GAME OVER", 250, 250, 50, RED);
            DrawText("Press M to return to Menu", 150, 350, 30, WHITE);

            if (IsKeyPressed(KEY_M))
            {
                jogador.vida.atual = jogador.vida.maxima;
                estado = MENU;
            }
        }

        EndDrawing();
    } // O loop principal termina AQUI

    // Limpeza de memória ao sair
    UnloadTexture(spriteInimigo);
    UnloadTexture(imagemPause);
    CloseWindow();

    return 0;
}
