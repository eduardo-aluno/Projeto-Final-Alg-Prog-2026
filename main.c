#include "raylib.h"
#include "inimigo.h"
#include "jogador.h"
#include "mapa.h"
#include <stdio.h>

#define MENU 0
#define JOGO 1
#define PAUSE 2
#define GAMEOVER 3
#define CREDITOS 4
#define AJUDA 5

void SalvarJogo(Jogador *jogador, int faseAtual) {
    FILE *arquivo = fopen("save.txt", "w");
    if (arquivo != NULL) {
        // Salva a vida atual, vida máxima e a fase atual
        fprintf(arquivo, "%d %d %d\n", jogador->vida.atual, jogador->vida.maxima, faseAtual);
        fclose(arquivo);
    }
}

bool CarregarJogo(Jogador *jogador, int *faseAtual) {
    FILE *arquivo = fopen("save.txt", "r");
    if (arquivo != NULL) {
        // Carrega os dados salvos
        fscanf(arquivo, "%d %d %d", &jogador->vida.atual, &jogador->vida.maxima, faseAtual);
        fclose(arquivo);
        return true; // Retorna verdadeiro se o save existir
    }
    return false; // Retorna falso se não existir save
}

void NovoJogo(Jogador *jogador, int *faseAtual) {
    jogador->vida.atual = 5;
    jogador->vida.maxima = 5;
    jogador->moedas = 0;           // NOVO
    jogador->energia = 0;          // NOVO
    jogador->energiaMaxima = 100;  // NOVO
    *faseAtual = 0; // 0 representa a Vila
    remove("save.txt"); // Apaga o save antigo, se houver
}

void SpawnarEntidadesDoMapa(Mapa mapa, Inimigo inimigos[], Texture2D texInimigo, Texture2D texBoss) {
    // Primeiro, limpa todos os inimigos
    for (int i = 0; i < MAX_INIMIGOS; i++) {
        inimigos[i].ativo = false;
    }

    int index = 0;

    // Varre o mapa procurando 'M' e 'C'
    for (int l = 0; l < MAPA_LINHAS; l++) {
        for (int c = 0; c < MAPA_COLUNAS; c++) {
            if (mapa.matriz[l][c] == 'M' && index < MAX_INIMIGOS) {
                inimigos[index].largura = 50;
                inimigos[index].altura = 50;
                inimigos[index].x = c * TAMANHO_BLOCO;

                // --- AJUSTE: Alinha exatamente em cima do bloco ---
                inimigos[index].y = (l * TAMANHO_BLOCO) + TAMANHO_BLOCO - inimigos[index].altura;

                inimigos[index].velocidadeX = 2.0f;
                inimigos[index].vida = 3;
                inimigos[index].ativo = true;
                inimigos[index].eChefe = false;
                inimigos[index].sprite = texInimigo;
                index++;
            }
            else if (mapa.matriz[l][c] == 'C' && index < MAX_INIMIGOS) {
                // --- AJUSTE: Calcula o Y compensando os 80px de altura do Boss ---
                float posYCorrigida = (l * TAMANHO_BLOCO) + TAMANHO_BLOCO - 80;
                InicializarBoss(&inimigos[index], c * TAMANHO_BLOCO, posYCorrigida, texBoss);
                index++;
            }
        }
    }
}

int main()
{
    InitWindow(800, 600, "Hollow Knight");
    SetExitKey(KEY_NULL);

    Texture2D imagemPause = LoadTexture("assets/image.pause.png");
    Texture2D spriteInimigo = LoadTexture("assets/inimigo.png");
    Texture2D spriteJogador = LoadTexture("assets/jogador.png");
    Texture2D spriteBoss = LoadTexture("assets/boss1.png");
    Texture2D spriteAtaque = LoadTexture("assets/guarana.png");

    SetTargetFPS(60);

    Jogador jogador;
    jogador.vida.atual = 5;
    jogador.vida.maxima = 5;
    jogador.moedas = 0;           // NOVO
    jogador.energia = 0;          // NOVO
    jogador.energiaMaxima = 100;  // NOVO
    int estado = MENU;
    int faseAtual = 0; // Variável para controlar a fase (0 = Vila, 1 = Tunel 1)
    jogador.velocidadeY = 0;
    jogador.largura = 50;
    jogador.altura = 50;
    jogador.atacando = false;
    jogador.tempoAtaque = 0;
    jogador.tempoInvencivel = 0;
    jogador.olhandoDireita = true;
    jogador.sprite = spriteJogador;

    Mapa mapaAtual = CarregarMapa("bin/Debug/vila.txt");
    InicializarPosicaoJogador(mapaAtual, &jogador);

    Inimigo inimigos[MAX_INIMIGOS];
    SpawnarEntidadesDoMapa(mapaAtual, inimigos, spriteInimigo, spriteBoss);;

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
        bool pertoDoPoco = false;

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
            AtualizarInimigos(inimigos, MAX_INIMIGOS, 0, MAPA_COLUNAS * TAMANHO_BLOCO, jogador.x);

            if (IsKeyPressed(KEY_UP) && noChao)
            {
                jogador.velocidadeY = -12;
            }

            // --- TRANSIÇÃO DE MAPA (O POÇO) ---
            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'E')
                    {
                        Rectangle rectPoco = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };

                        // O poço SÓ FUNCIONA se o jogador estiver encostando nele E estiver na Vila (faseAtual == 0)
                        if (CheckCollisionRecs(rectJogador, rectPoco) && faseAtual == 0)
                        {
                            pertoDoPoco = true; // Avisa o loop de desenho que pode mostrar o texto

                            if (IsKeyPressed(KEY_UP))
                            {
                                // Vai para o Túnel
                                faseAtual = 1;
                                mapaAtual = CarregarMapa("bin/Debug/o_teste.txt"); // Troque pelo nome do seu tunel1

                                // Reposiciona o jogador e spawna os inimigos para o novo mapa
                                InicializarPosicaoJogador(mapaAtual, &jogador);
                                SpawnarEntidadesDoMapa(mapaAtual, inimigos, spriteInimigo, spriteBoss);
                            }
                        }
                    }
                }
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

                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    if (inimigos[i].ativo)
                    {
                        Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };
                        if (CheckCollisionRecs(rectAtaque, rectInimigo))
                        {
                            inimigos[i].vida -= 1;

                            // --- NOVO: Ganha energia a cada acerto! ---
                            jogador.energia += 20;
                            if (jogador.energia > jogador.energiaMaxima) {
                                jogador.energia = jogador.energiaMaxima;
                            }
                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;

                                // Ganha moedas (Chefe dá 5, normal dá 1)
                                jogador.moedas += inimigos[i].eChefe ? 5 : 1;

                                // --- NOVO: SE O CHEFE MORRER, VOLTA PRA VILA ---
                                if (inimigos[i].eChefe)
                                {
                                    faseAtual = 0; // 0 é a Vila
                                    mapaAtual = CarregarMapa("bin/Debug/vila.txt");

                                    // Regenera a vida do jogador ao voltar
                                    jogador.vida.atual = jogador.vida.maxima;

                                    // Reposiciona na Vila e reseta as entidades
                                    InicializarPosicaoJogador(mapaAtual, &jogador);
                                    SpawnarEntidadesDoMapa(mapaAtual, inimigos, spriteInimigo, spriteBoss);
                                }
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

            // --- NOVO: CURA (Gasta 50 de energia apertando A) ---
            if (IsKeyPressed(KEY_A))
            {
                // Se tem energia suficiente e não está com a vida cheia
                if (jogador.energia >= 50 && jogador.vida.atual < jogador.vida.maxima)
                {
                    jogador.energia -= 50;
                    jogador.vida.atual += 1;
                }
            }

// --- SISTEMA DE COMBATE: INIMIGO DANDO DANO NO JOGADOR ---
            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (inimigos[i].ativo)
                {
                    Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

                    if (CheckCollisionRecs(rectJogador, rectInimigo) && jogador.tempoInvencivel == 0)
                    {
                        // Se for chefe tira 2, se for normal tira 1
                        int dano = inimigos[i].eChefe ? 2 : 1;
                        ReceberDano(&jogador, dano);
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
            DrawText("J - Jogar (Continuar)", 250, 300, 30, WHITE);
            DrawText("N - Novo Jogo", 250, 350, 30, WHITE);
            DrawText("A - Ajuda (Tutorial)", 250, 400, 30, YELLOW);
            DrawText("C - Creditos", 250, 450, 30, WHITE);
            DrawText("S - Sair", 250, 500, 30, WHITE);

            // Carrega o jogo salvo
            if (IsKeyPressed(KEY_J))
            {
                if (!CarregarJogo(&jogador, &faseAtual)) {
                    NovoJogo(&jogador, &faseAtual); // Se não achar o arquivo, cria um novo jogo
                }
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = JOGO;
            }

            // Inicia um novo jogo do zero
            if (IsKeyPressed(KEY_N))
            {
                NovoJogo(&jogador, &faseAtual);
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = JOGO;
            }
            if (IsKeyPressed(KEY_A))
            {
                estado = AJUDA;
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
            DesenharInimigos(inimigos, MAX_INIMIGOS);

            Rectangle sourceRec = { 0.0f, 0.0f, (float)jogador.sprite.width, (float)jogador.sprite.height };
            Rectangle destRec = { jogador.x, jogador.y, (float)jogador.largura, (float)jogador.altura };
            Vector2 origin = { 0.0f, 0.0f };

            // Faz o jogador piscar quando estiver invencível
            if (jogador.tempoInvencivel == 0 || (jogador.tempoInvencivel / 5) % 2 == 0)
            {
                DrawTexturePro(jogador.sprite, sourceRec, destRec, origin, 0.0f, WHITE);
            }

            // O ataque desenhado dependendo do lado para onde olha
            // --- DESENHO DO ATAQUE (Guaraná) ---
            if (jogador.atacando)
            {
                // Pega a imagem inteira do guaraná
                Rectangle sourceRec = { 0.0f, 0.0f, (float)spriteAtaque.width, (float)spriteAtaque.height };
                Rectangle destRec;

                if (jogador.olhandoDireita)
                {
                    // Desenha do lado direito do jogador (Tamanho 40x40 para não ficar muito esticado)
                    destRec = (Rectangle){ jogador.x + jogador.largura, jogador.y + 10, 40, 40 };
                    DrawTexturePro(spriteAtaque, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
                }
                else
                {
                    // Inverte a largura do sourceRec para espelhar a imagem virada para a esquerda
                    sourceRec.width = -sourceRec.width;
                    destRec = (Rectangle){ jogador.x - 40, jogador.y + 10, 40, 40 };
                    DrawTexturePro(spriteAtaque, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
                }
            } // <--- AQUI ESTÁ A CHAVE QUE FALTAVA! FECHANDO O ATAQUE!

            // --- DESENHO DO POÇO ---
            if (pertoDoPoco)
            {
                DrawText("Aperte CIMA para entrar/sair", jogador.x - 60, jogador.y - 30, 15, WHITE);
            }

            EndMode2D(); // Finaliza a câmera 2D

            DesenharHUD(jogador); // Desenha a interface (vida, moedas) por cima de tudo

            DrawText("Press S to attack", 350, 550, 20, RED);
            DrawText("Press ESC to Pause", 20, 550, 20, WHITE);
        }
        else if (estado == PAUSE)
        {
            DrawTexture(imagemPause, 0, 0, WHITE);

            DrawText("GAME PAUSED", 230, 200, 40, MAROON);
            DrawText("Press C to Continue", 180, 300, 30, MAROON);
            DrawText("Press J to Save Game", 180, 350, 30, MAROON);
            DrawText("Press S to Exit to Menu", 150, 400, 30, MAROON);

            if (IsKeyPressed(KEY_C))
                estado = JOGO;

            // Salva o jogo e cria o arquivo save.txt na pasta do projeto
            if (IsKeyPressed(KEY_J))
            {
                SalvarJogo(&jogador, faseAtual);
                DrawText("JOGO SALVO!", 300, 450, 30, GREEN); // Feedback visual rápido
            }

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
        else if (estado == AJUDA)
        {
            DrawText("TUTORIAL E CONTROLES", 180, 50, 40, YELLOW);

            DrawText("- Setas Direita/Esquerda: Movimentam o personagem", 50, 150, 20, WHITE);
            DrawText("- Seta Cima: Pular / Entrar no Poco (Transicoes)", 50, 190, 20, WHITE);
            DrawText("- Tecla 'S': Atacar inimigos com o Guarana", 50, 230, 20, WHITE);
            DrawText("- Tecla 'A': Curar 1 de vida (Gasta 50 de Energia)", 50, 270, 20, WHITE);
            DrawText("- Tecla 'ESC': Pausar o jogo", 50, 310, 20, WHITE);

            DrawText("MECANICAS DO JOGO:", 50, 370, 25, GREEN);
            DrawText("* Derrote inimigos para ganhar energia e moedas.", 50, 410, 20, LIGHTGRAY);
            DrawText("* O Chefe da fase tira 2 de vida e fica no final do mapa.", 50, 440, 20, LIGHTGRAY);
            DrawText("* Ao derrotar o chefe, voce volta para a Vila recuperado!", 50, 470, 20, LIGHTGRAY);

            DrawText("Pressione ESC para voltar ao Menu", 200, 540, 20, GRAY);

            // Voltar para o menu
            if (IsKeyPressed(KEY_ESCAPE))
            {
                estado = MENU;
            }
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
    UnloadTexture(spriteAtaque);
    UnloadTexture(imagemPause);
    CloseWindow();

    return 0;
}
