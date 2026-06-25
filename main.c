#include "raylib.h"
#include "inimigo.h"
#include "jogador.h"
#include "mapa.h"
#include "plataforma.h"
#include <stdio.h>

#define MENU 0
#define JOGO 1
#define PAUSE 2
#define GAMEOVER 3
#define CREDITOS 4
#define AJUDA 5
#define INVENTARIO 6
#define VITORIA 7

void SalvarJogo(Jogador *jogador, int faseAtual)
{
    FILE *arquivo = fopen("save.txt", "w");
    if (arquivo != NULL)
    {
        fprintf(arquivo, "%d %d %d\n", jogador->vida.atual, jogador->vida.maxima, faseAtual);
        fclose(arquivo);
    }
}

bool CarregarJogo(Jogador *jogador, int *faseAtual)
{
    FILE *arquivo = fopen("save.txt", "r");
    if (arquivo != NULL)
    {
        fscanf(arquivo, "%d %d %d", &jogador->vida.atual, &jogador->vida.maxima, faseAtual);
        fclose(arquivo);
        return true;
    }
    return false;
}

void NovoJogo(Jogador *jogador, int *faseAtual)
{
    jogador->vida.atual = 5;
    jogador->vida.maxima = 5;
    jogador->moedas = 0;
    jogador->energia = 0;
    jogador->energiaMaxima = 100;
    *faseAtual = 0;
    remove("save.txt");
}

void SpawnarEntidadesDoMapa(Mapa mapa, Inimigo inimigos[], Texture2D texInimigo, Texture2D texBoss)
{
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].ativo = false;
    }

    int index = 0;

    for (int l = 0; l < MAPA_LINHAS; l++)
    {
        for (int c = 0; c < MAPA_COLUNAS; c++)
        {
            if (mapa.matriz[l][c] == 'M' && index < MAX_INIMIGOS)
            {
                inimigos[index].largura = 50;
                inimigos[index].altura = 50;
                inimigos[index].x = c * TAMANHO_BLOCO;
                inimigos[index].y = (l * TAMANHO_BLOCO) + TAMANHO_BLOCO - inimigos[index].altura;
                inimigos[index].velocidadeX = 2.0f;
                inimigos[index].vida = 3;
                inimigos[index].ativo = true;
                inimigos[index].eChefe = false;
                inimigos[index].sprite = texInimigo;
                index++;
            }
            else if (mapa.matriz[l][c] == 'C' && index < MAX_INIMIGOS)
            {
                float posYCorrigida = (l * TAMANHO_BLOCO) + TAMANHO_BLOCO - 80;
                InicializarBoss(&inimigos[index], c * TAMANHO_BLOCO, posYCorrigida, texBoss);
                index++;
            }
        }
    }
}

void MudarDeFase(int fase, Mapa *mapa, Jogador *jogador, Inimigo inimigos[], Texture2D texInimigo, Texture2D texBoss, PlataformaMovel plataformas[])
{
    Mapa novoMapa;

    switch (fase)
    {
    case 0:  novoMapa = CarregarMapa("bin/Debug/vila.txt");     break;
    case 1:  novoMapa = CarregarMapa("bin/Debug/map - 1.txt");  break;
    case 2:  novoMapa = CarregarMapa("bin/Debug/map - 2.txt");  break;
    case 3:  novoMapa = CarregarMapa("bin/Debug/map - 3.txt");  break;
    default: novoMapa = CarregarMapa("bin/Debug/vila.txt");     break;
    }

    InicializarPosicaoJogador(novoMapa, jogador);
    SpawnarEntidadesDoMapa(novoMapa, inimigos, texInimigo, texBoss);
    SpawnarPlataformas(novoMapa, plataformas);

    *mapa = novoMapa;
}

int main()
{
    InitWindow(1200, 800, "Hollow Knight");
    InitAudioDevice();
    SetExitKey(KEY_NULL);

    Texture2D imagemPause = LoadTexture("assets/image.pause.png");
    Texture2D spriteInimigo = LoadTexture("assets/inimigo.png");
    Texture2D spriteJogador = LoadTexture("assets/jogador.png");
    Texture2D spriteBoss = LoadTexture("assets/boss1.png");
    Texture2D spriteAtaque = LoadTexture("assets/guarana.png");
    Texture2D texturaChao = LoadTexture("assets/lestone.png");
    Texture2D fundo = LoadTexture("assets/plano-de-fundo.png");
    Texture2D imagemCheat = LoadTexture("assets/imagemCheat.png");

    Sound somAtaque = LoadSound("assets/GUARANAA.mp3");
    Sound somDano = LoadSound("assets/SHAW.mp3");
    Sound somCheat = LoadSound("assets/cheat-code-audio.mp3");

    SetTexturaChao(texturaChao);
    SetTargetFPS(60);

    Jogador jogador;
    InicializarJogador(&jogador);
    jogador.sprite = spriteJogador;

    int estado = MENU;
    int cliquesTeclaY = 0;
    bool cheatVidaInfinitaAtivo = false;
    bool exibindoTelaCheat = false;
    int slotSelecionado = 0;
    int precoAmuleto = 15;
    int faseAtual = 0;
    int proximaFase = 1;
    bool emFase = false;

    bool mostrarFeedbackAmuleto = false;
    int tempoFeedbackAmuleto = 0;

    Mapa mapaAtual = CarregarMapa("bin/Debug/vila.txt");
    InicializarPosicaoJogador(mapaAtual, &jogador);

    Inimigo inimigos[MAX_INIMIGOS];
    SpawnarEntidadesDoMapa(mapaAtual, inimigos, spriteInimigo, spriteBoss);

    PlataformaMovel plataformas[MAX_PLATAFORMAS];
    SpawnarPlataformas(mapaAtual, plataformas);

    Camera2D camera = { 0 };
    camera.target = (Vector2){ jogador.x, jogador.y };
    camera.offset = (Vector2){ 600.0f, 400.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose())
    {
        bool pertoDoPoco = false;

        // ==========================================
        // 1. LÓGICA DO JOGO
        // ==========================================
        if (estado == JOGO)
        {
            if (cheatVidaInfinitaAtivo)
            {
                jogador.vida.atual = jogador.vida.maxima;
            }

            if (IsKeyPressed(KEY_Y))
            {
                cliquesTeclaY++;
                if (cliquesTeclaY >= 3 && !cheatVidaInfinitaAtivo)
                {
                    cheatVidaInfinitaAtivo = true;
                    exibindoTelaCheat = true;
                    estado = PAUSE;
                    PlaySound(somCheat);
                }
            }

            if (jogador.tempoInvencivel > 0) jogador.tempoInvencivel--;

            // --- MOVIMENTAÇÃO EIXO X ---
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

            for (int i = 0; i < MAPA_LINHAS; i++)
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

            // --- MOVIMENTAÇÃO EIXO Y ---
            jogador.velocidadeY += 0.5f;
            jogador.y += jogador.velocidadeY;
            rectJogador.y = jogador.y;

            bool estaNoChao = false;

            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'P')
                    {
                        Rectangle rectBloco = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };
                        if (CheckCollisionRecs(rectJogador, rectBloco))
                        {
                            if (jogador.velocidadeY > 0)
                            {
                                jogador.y = rectBloco.y - jogador.altura;
                                jogador.velocidadeY = 0;
                                estaNoChao = true;
                            }
                            else if (jogador.velocidadeY < 0)
                            {
                                jogador.y = rectBloco.y + TAMANHO_BLOCO;
                                jogador.velocidadeY = 0;
                            }
                            rectJogador.y = jogador.y;
                        }
                    }
                }
            }

            AtualizarPlataformas(plataformas, &jogador, &estaNoChao);

            camera.target = (Vector2){ jogador.x, jogador.y };

            AtualizarInimigos(inimigos, MAX_INIMIGOS, 0, MAPA_COLUNAS * TAMANHO_BLOCO, &jogador, mapaAtual);
            AtualizarCura(&jogador);
            AtualizarDash(&jogador);

            if (IsKeyPressed(KEY_D))
            {
                AtivarDash(&jogador);
            }

            if (IsKeyPressed(KEY_UP))
            {
                if (estaNoChao)
                {
                    jogador.velocidadeY = -12;
                    jogador.puloDuploUsado = false;
                }
                else if (jogador.puloDuploDisponivel && !jogador.puloDuploUsado)
                {
                    jogador.velocidadeY = -10;
                    jogador.puloDuploUsado = true;
                }
            }

            // --- COLETA DE AMULETOS NO MAPA ---
            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'H')
                    {
                        Rectangle rectAmuleto = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };
                        if (CheckCollisionRecs(rectJogador, rectAmuleto))
                        {
                            jogador.possuiAmuletoEspecial = true;
                            jogador.amuletoAtaqueEspecial = true;
                            mapaAtual.matriz[i][j] = ' ';
                            mostrarFeedbackAmuleto = true;
                            tempoFeedbackAmuleto = 120;
                        }
                    }
                }
            }

            // --- ATAQUE ESPECIAL (HADOUKEN) ---
            if (IsKeyPressed(KEY_Z) && jogador.amuletoAtaqueEspecial && jogador.energia >= 30 && !jogador.projetilAtivo)
            {
                jogador.projetilAtivo = true;
                jogador.projetilX = jogador.x + (jogador.olhandoDireita ? jogador.largura : -20);
                jogador.projetilY = jogador.y + 20;
                jogador.projetilVelocidadeX = jogador.olhandoDireita ? 10.0f : -10.0f;
                jogador.projetilDano = 5;
                jogador.energia -= 30;
                jogador.ataqueEspecialAtivo = true;
                jogador.tempoAtaqueEspecial = 30;
            }

            // Atualiza o projétil ativo
            if (jogador.projetilAtivo)
            {
                jogador.projetilX += jogador.projetilVelocidadeX;

                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    if (inimigos[i].ativo)
                    {
                        Rectangle rectProjetil = { jogador.projetilX, jogador.projetilY, 20, 20 };
                        Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };

                        if (CheckCollisionRecs(rectProjetil, rectInimigo))
                        {
                            if (inimigos[i].eChefe)
                            {
                                inimigos[i].vida -= inimigos[i].vida / 2;
                            }
                            else
                            {
                                inimigos[i].vida -= jogador.projetilDano;
                            }

                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;
                                jogador.moedas += inimigos[i].eChefe ? 5 : 1;

                                if (inimigos[i].eChefe)
                                {
                                    if (proximaFase < 3)
                                    {
                                        proximaFase++;
                                        faseAtual = 0;
                                        emFase = false;
                                        jogador.vida.atual = jogador.vida.maxima;
                                        MudarDeFase(faseAtual, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss, plataformas);
                                    }
                                    else
                                    {
                                        estado = VITORIA;
                                    }
                                }
                            }
                            jogador.projetilAtivo = false;
                            break;
                        }
                    }
                }

                if (jogador.projetilX > MAPA_COLUNAS * TAMANHO_BLOCO || jogador.projetilX < 0)
                {
                    jogador.projetilAtivo = false;
                }
            }

            if (jogador.ataqueEspecialAtivo)
            {
                jogador.tempoAtaqueEspecial--;
                if (jogador.tempoAtaqueEspecial <= 0)
                {
                    jogador.ataqueEspecialAtivo = false;
                }
            }

            // --- TRANSIÇÃO DE MAPA (O POÇO) ---
            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'E')
                    {
                        Rectangle rectPoco = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };
                        if (CheckCollisionRecs(rectJogador, rectPoco) && faseAtual == 0)
                        {
                            pertoDoPoco = true;
                            if (IsKeyPressed(KEY_UP))
                            {
                                faseAtual = proximaFase;
                                emFase = true;
                                MudarDeFase(faseAtual, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss, plataformas);
                            }
                        }
                    }
                }
            }

            // --- CONTROLE DE ATAQUE PADRÃO ---
            if (IsKeyPressed(KEY_S) && !jogador.atacando)
            {
                jogador.atacando = true;
                jogador.tempoAtaque = 15;
                PlaySound(somAtaque);

                Rectangle rectAtaque;
                if (jogador.olhandoDireita)
                {
                    rectAtaque = (Rectangle){ jogador.x + jogador.largura, jogador.y + 10, 40, 30 };
                }
                else
                {
                    rectAtaque = (Rectangle){ jogador.x - 40, jogador.y + 10, 40, 30 };
                }

                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    if (inimigos[i].ativo)
                    {
                        Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };
                        if (CheckCollisionRecs(rectAtaque, rectInimigo))
                        {
                            inimigos[i].vida -= jogador.danoAtaque;
                            jogador.energia += 20;
                            if (jogador.energia > jogador.energiaMaxima) jogador.energia = jogador.energiaMaxima;

                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;
                                jogador.moedas += inimigos[i].eChefe ? 5 : 1;

                                if (inimigos[i].eChefe)
                                {
                                    if (proximaFase < 3)
                                    {
                                        proximaFase++;
                                        faseAtual = 0;
                                        emFase = false;
                                        jogador.vida.atual = jogador.vida.maxima;
                                        MudarDeFase(faseAtual, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss, plataformas);
                                    }
                                    else
                                    {
                                        estado = VITORIA;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (jogador.atacando)
            {
                jogador.tempoAtaque--;
                if (jogador.tempoAtaque <= 0) jogador.atacando = false;
            }

            // --- DANO NO JOGADOR ---
            for (int i = 0; i < MAX_INIMIGOS; i++)
            {
                if (inimigos[i].ativo)
                {
                    Rectangle rectInimigo = { inimigos[i].x, inimigos[i].y, inimigos[i].largura, inimigos[i].altura };
                    if (CheckCollisionRecs(rectJogador, rectInimigo) && jogador.tempoInvencivel == 0)
                    {
                        int dano = inimigos[i].eChefe ? 2 : 1;
                        ReceberDano(&jogador, dano);
                        jogador.tempoInvencivel = 60;
                        PlaySound(somDano);

                        if (jogador.vida.atual <= 0) estado = GAMEOVER;
                    }
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) estado = PAUSE;
            if (IsKeyPressed(KEY_I))      estado = INVENTARIO;
        }

        // ==========================================
        // LÓGICA DO INVENTÁRIO
        // ==========================================
        else if (estado == INVENTARIO)
        {
            if (IsKeyPressed(KEY_ESCAPE)) estado = JOGO;

            if (IsKeyPressed(KEY_RIGHT))
            {
                slotSelecionado++;
                if (slotSelecionado > 3) slotSelecionado = 0;
            }
            if (IsKeyPressed(KEY_LEFT))
            {
                slotSelecionado--;
                if (slotSelecionado < 0) slotSelecionado = 3;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                jogador.amuletoVidaExtra = false;
                jogador.amuletoDanoExtra = false;
                jogador.amuletoCuraEficiente = false;

                if (slotSelecionado == 0)
                {
                    if (jogador.possuiAmuletoVida) jogador.amuletoVidaExtra = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoVida = true;
                        jogador.amuletoVidaExtra = true;
                    }
                }
                else if (slotSelecionado == 1)
                {
                    if (jogador.possuiAmuletoDano) jogador.amuletoDanoExtra = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoDano = true;
                        jogador.amuletoDanoExtra = true;
                    }
                }
                else if (slotSelecionado == 2)
                {
                    if (jogador.possuiAmuletoCura) jogador.amuletoCuraEficiente = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoCura = true;
                        jogador.amuletoCuraEficiente = true;
                    }
                }
                else if (slotSelecionado == 3)
                {
                    if (jogador.possuiAmuletoEspecial)
                    {
                        jogador.amuletoAtaqueEspecial = !jogador.amuletoAtaqueEspecial;
                    }
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoEspecial = true;
                        jogador.amuletoAtaqueEspecial = true;
                    }
                }

                AplicarAmuletos(&jogador);
            }
        }

        // ==========================================
        // 2. DESENHO DO JOGO (RENDERIZAÇÃO)
        // ==========================================
        BeginDrawing();
        ClearBackground(BLACK);

        if (estado == MENU)
        {
            DrawText("HOLLOW KNIGHT", 450, 250, 40, WHITE);
            DrawText("J - Jogar (Continuar)", 450, 350, 30, WHITE);
            DrawText("N - Novo Jogo", 450, 400, 30, WHITE);
            DrawText("A - Ajuda (Tutorial)", 450, 450, 30, YELLOW);
            DrawText("C - Creditos", 450, 500, 30, WHITE);
            DrawText("S - Sair", 450, 550, 30, WHITE);

            if (IsKeyPressed(KEY_J))
            {
                if (!CarregarJogo(&jogador, &faseAtual)) NovoJogo(&jogador, &faseAtual);
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = JOGO;
            }
            if (IsKeyPressed(KEY_N))
            {
                NovoJogo(&jogador, &faseAtual);
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = JOGO;
            }
            if (IsKeyPressed(KEY_A)) estado = AJUDA;
            if (IsKeyPressed(KEY_S)) CloseWindow();
            if (IsKeyPressed(KEY_C)) estado = CREDITOS;
        }
        else if (estado == JOGO)
        {
            Rectangle sourceRecFundo = { 0.0f, 0.0f, (float)fundo.width, (float)fundo.height };
            Rectangle destRecFundo = { 0.0f, 0.0f, 1200.0f, 800.0f };
            Vector2 originFundo = { 0.0f, 0.0f };

            DrawTexturePro(fundo, sourceRecFundo, destRecFundo, originFundo, 0.0f, WHITE);

            BeginMode2D(camera);
            DesenharMapa(mapaAtual);
            DesenharInimigos(inimigos, MAX_INIMIGOS);
            DesenharPlataformas(plataformas, texturaChao);

            Rectangle sourceRec = { 0.0f, 0.0f, (float)jogador.sprite.width, (float)jogador.sprite.height };
            Rectangle destRec = { jogador.x, jogador.y, (float)jogador.largura, (float)jogador.altura };
            Vector2 origin = { 0.0f, 0.0f };

            if (jogador.tempoInvencivel == 0 || (jogador.tempoInvencivel / 5) % 2 == 0)
            {
                DrawTexturePro(jogador.sprite, sourceRec, destRec, origin, 0.0f, WHITE);
            }

            if (jogador.atacando)
            {
                Rectangle sourceRecAtaque = { 0.0f, 0.0f, (float)spriteAtaque.width, (float)spriteAtaque.height };
                Rectangle destRecAtaque;

                if (jogador.olhandoDireita)
                {
                    destRecAtaque = (Rectangle){ jogador.x + jogador.largura, jogador.y + 10, 40, 40 };
                    DrawTexturePro(spriteAtaque, sourceRecAtaque, destRecAtaque, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }
                else
                {
                    sourceRecAtaque.width = -sourceRecAtaque.width;
                    destRecAtaque = (Rectangle){ jogador.x - 40, jogador.y + 10, 40, 40 };
                    DrawTexturePro(spriteAtaque, sourceRecAtaque, destRecAtaque, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }
            }

            if (pertoDoPoco)
            {
                DrawText("Aperte CIMA para entrar/sair", jogador.x - 60, jogador.y - 30, 15, WHITE);
            }

            // --- PROJÉTIL AZUL DO ATAQUE ESPECIAL ---
            if (jogador.projetilAtivo)
            {
                DrawCircle(jogador.projetilX + 10, jogador.projetilY + 10, 15, BLUE);
                DrawCircle(jogador.projetilX + 10, jogador.projetilY + 10, 10, SKYBLUE);
                DrawCircleLines(jogador.projetilX + 10, jogador.projetilY + 10, 15, DARKBLUE);
            }

            EndMode2D();

            DesenharHUD(jogador);

            // Interface limpa de botões na HUD
            DrawText("S - Atacar", 25, 715, 16, RED);
            DrawText("Z - Tiro Especial", 25, 740, 16, SKYBLUE);
            DrawText("D - Dash", 25, 765, 16, BLUE);

            DrawText("A (segure) - Curar", 280, 715, 16, GREEN);
            DrawText("Seta para Cima - Pulo", 280, 740, 16, YELLOW);
            DrawText("ESC - Pausar Jogo", 280, 765, 16, WHITE);

            char textoFase[50];
            sprintf(textoFase, "Fase: %d/%d", proximaFase-1, 2);
            DrawText(textoFase, 1100 - MeasureText(textoFase, 20), 20, 20, GOLD);
        }
        else if (estado == PAUSE)
        {
            if (exibindoTelaCheat)
            {
                ClearBackground(BLACK);
                DrawTexture(imagemCheat, 450, 100, WHITE);
                DrawText("CHEAT CODE ATIVADO - NOOB DETECTADO", 250, 580, 35, RED);
                DrawText("clique em enter para continuar", 430, 640, 20, LIGHTGRAY);

                if (IsKeyPressed(KEY_ENTER))
                {
                    exibindoTelaCheat = false;
                    estado = JOGO;
                    cliquesTeclaY = 0;
                }
            }
            else
            {
                DrawTexture(imagemPause, 0, 0, WHITE);
                DrawText("GAME PAUSED", 460, 220, 40, MAROON);
                DrawText("Press C to Continue", 440, 340, 30, MAROON);
                DrawText("Press J to Save Game", 435, 400, 30, MAROON);
                DrawText("Press S to Exit to Menu", 410, 460, 30, MAROON);

                if (IsKeyPressed(KEY_C)) estado = JOGO;
                if (IsKeyPressed(KEY_S)) estado = MENU;
                if (IsKeyPressed(KEY_J)) SalvarJogo(&jogador, faseAtual);
            }
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

            if (IsKeyPressed(KEY_ESCAPE)) estado = MENU;
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

            if (IsKeyPressed(KEY_ESCAPE)) estado = MENU;
        }
        else if (estado == GAMEOVER)
        {
            DrawText("GAME OVER", 460, 300, 50, RED);
            DrawText("Press M to return to Menu", 410, 400, 30, WHITE);

            if (IsKeyPressed(KEY_M))
            {
                jogador.vida.atual = jogador.vida.maxima;
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = MENU;
            }
        }
        else if (estado == VITORIA)
        {
            ClearBackground(BLACK);
            DrawText("VOCE VENCEU!", 400, 200, 60, GOLD);
            DrawText("PARABENS POR COMPLETAR O JOGO!", 280, 300, 35, WHITE);
            DrawText(TextFormat("Moedas coletadas: %d", jogador.moedas), 430, 380, 25, YELLOW);
            DrawText("Pressione ENTER para voltar ao Menu", 350, 500, 25, LIGHTGRAY);

            if (IsKeyPressed(KEY_ENTER))
            {
                estado = MENU;
                proximaFase = 1;
                faseAtual = 0;
                MudarDeFase(0, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss, plataformas);
            }
        }
        else if (estado == INVENTARIO)
        {
            ClearBackground(BLACK);
            DrawText("INVENTARIO DE AMULETOS", 400, 100, 30, WHITE);
            DrawText("Pressione ESC para fechar e voltar", 430, 150, 18, GRAY);
            DrawText(TextFormat("Suas Moedas: %d", jogador.moedas), 520, 200, 22, YELLOW);

            for (int i = 0; i < 4; i++)
            {
                int posX = 150 + (i * 220);
                int posY = 320;

                Color corBorda = (i == slotSelecionado) ? YELLOW : WHITE;
                DrawRectangleLines(posX, posY, 200, 200, corBorda);
                if (i == slotSelecionado)
                {
                    DrawRectangleLines(posX - 2, posY - 2, 204, 204, YELLOW);
                }

                char* nome = "";
                char* desc = "";
                bool possui = false;
                bool ativo = false;

                if (i == 0)
                {
                    nome = "Amuleto de Vida";
                    desc = "+2 Coracoes Max";
                    possui = jogador.possuiAmuletoVida;
                    ativo = jogador.amuletoVidaExtra;
                }
                else if (i == 1)
                {
                    nome = "Amuleto de Dano";
                    desc = "+1 Dano de Ataque";
                    possui = jogador.possuiAmuletoDano;
                    ativo = jogador.amuletoDanoExtra;
                }
                else if (i == 2)
                {
                    nome = "Amuleto de Cura";
                    desc = "Cura mais rapida\ne barata";
                    possui = jogador.possuiAmuletoCura;
                    ativo = jogador.amuletoCuraEficiente;
                }
                else if (i == 3)
                {
                    nome = "Amuleto Especial";
                    desc = "Dispara projeteis\nazuis (Tecla Z)";
                    possui = jogador.possuiAmuletoEspecial;
                    ativo = jogador.amuletoAtaqueEspecial;
                }

                DrawText(nome, posX + 15, posY + 20, 18, WHITE);
                DrawText(desc, posX + 15, posY + 60, 14, LIGHTGRAY);

                if (ativo)       DrawText("EQUIPADO", posX + 50, posY + 140, 16, GREEN);
                else if (possui) DrawText("POSSUI", posX + 65, posY + 140, 16, BLUE);
                else
                {
                    if (faseAtual == 0) DrawText("COMPRAR: 15$ [Enter]", posX + 15, posY + 140, 14, ORANGE);
                    else                DrawText("BLOQUEADO", posX + 50, posY + 140, 16, RED);
                }
            }

            if (IsKeyPressed(KEY_ENTER))
            {
                jogador.amuletoVidaExtra = false;
                jogador.amuletoDanoExtra = false;
                jogador.amuletoCuraEficiente = false;

                if (slotSelecionado == 0)
                {
                    if (jogador.possuiAmuletoVida) jogador.amuletoVidaExtra = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoVida = true;
                        jogador.amuletoVidaExtra = true;
                    }
                }
                else if (slotSelecionado == 1)
                {
                    if (jogador.possuiAmuletoDano) jogador.amuletoDanoExtra = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoDano = true;
                        jogador.amuletoDanoExtra = true;
                    }
                }
                else if (slotSelecionado == 2)
                {
                    if (jogador.possuiAmuletoCura) jogador.amuletoCuraEficiente = true;
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoCura = true;
                        jogador.amuletoCuraEficiente = true;
                    }
                }
                else if (slotSelecionado == 3)
                {
                    if (jogador.possuiAmuletoEspecial)
                    {
                        jogador.amuletoAtaqueEspecial = !jogador.amuletoAtaqueEspecial;
                    }
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoEspecial = true;
                        jogador.amuletoAtaqueEspecial = true;
                    }
                }

                AplicarAmuletos(&jogador);
            }
        }

        EndDrawing();
    }

    UnloadTexture(spriteInimigo);
    UnloadTexture(spriteJogador);
    UnloadTexture(spriteAtaque);
    UnloadTexture(imagemPause);
    UnloadTexture(fundo);
    UnloadTexture(imagemCheat);

    UnloadSound(somAtaque);
    UnloadSound(somDano);
    UnloadSound(somCheat);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
