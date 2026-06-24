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
#define INVENTARIO 6

// ... (Suas funções SalvarJogo, CarregarJogo, NovoJogo, SpawnarEntidadesDoMapa, MudarDeFase permanecem IGUAIS) ...

void SalvarJogo(Jogador *jogador, int faseAtual)
{
    FILE *arquivo = fopen("save.txt", "w");
    if (arquivo != NULL)
    {
        // Salva a vida atual, vida máxima e a fase atual
        fprintf(arquivo, "%d %d %d\n", jogador->vida.atual, jogador->vida.maxima, faseAtual);
        fclose(arquivo);
    }
}

bool CarregarJogo(Jogador *jogador, int *faseAtual)
{
    FILE *arquivo = fopen("save.txt", "r");
    if (arquivo != NULL)
    {
        // Carrega os dados salvos
        fscanf(arquivo, "%d %d %d", &jogador->vida.atual, &jogador->vida.maxima, faseAtual);
        fclose(arquivo);
        return true; // Retorna verdadeiro se o save existir
    }
    return false; // Retorna falso se não existir save
}

void NovoJogo(Jogador *jogador, int *faseAtual)
{
    jogador->vida.atual = 5;
    jogador->vida.maxima = 5;
    jogador->moedas = 0;           // NOVO
    jogador->energia = 0;          // NOVO
    jogador->energiaMaxima = 100;  // NOVO
    *faseAtual = 0; // 0 representa a Vila
    remove("save.txt"); // Apaga o save antigo, se houver
}

void SpawnarEntidadesDoMapa(Mapa mapa, Inimigo inimigos[], Texture2D texInimigo, Texture2D texBoss)
{
    // Primeiro, limpa todos os inimigos
    for (int i = 0; i < MAX_INIMIGOS; i++)
    {
        inimigos[i].ativo = false;
    }

    int index = 0;

    // Varre o mapa procurando 'M' e 'C'
    for (int l = 0; l < MAPA_LINHAS; l++)
    {
        for (int c = 0; c < MAPA_COLUNAS; c++)
        {
            if (mapa.matriz[l][c] == 'M' && index < MAX_INIMIGOS)
            {
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
            else if (mapa.matriz[l][c] == 'C' && index < MAX_INIMIGOS)
            {
                // --- AJUSTE: Calcula o Y compensando os 80px de altura do Boss ---
                float posYCorrigida = (l * TAMANHO_BLOCO) + TAMANHO_BLOCO - 80;
                InicializarBoss(&inimigos[index], c * TAMANHO_BLOCO, posYCorrigida, texBoss);
                index++;
            }
        }
    }
}


void MudarDeFase(int fase, Mapa *mapa, Jogador *jogador, Inimigo inimigos[], Texture2D texInimigo, Texture2D texBoss)
{
    Mapa novoMapa;

    switch (fase)
    {
    case 0:
        novoMapa = CarregarMapa("bin/Debug/vila.txt");
        break;

    case 1:
        novoMapa = CarregarMapa("bin/Debug/map - 1.txt");
        break;

    case 2:
        novoMapa = CarregarMapa("bin/Debug/map - 2.txt");
        break;

    default:
        // Caso dê algum erro ou chegue ao fim das fases, volta para a vila em segurança
        novoMapa = CarregarMapa("bin/Debug/vila.txt");
        break;
    }

    // Agora passamos 'novoMapa' diretamente (sem o asterisco) para as funções!
    InicializarPosicaoJogador(novoMapa, jogador);
    SpawnarEntidadesDoMapa(novoMapa, inimigos, texInimigo, texBoss);

    // Por fim, guardamos o novo mapa de volta no ponteiro original do jogo
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
    int slotSelecionado = 0; // 0: Vida, 1: Dano, 2: Cura (Controla o menu de caixas)
    int precoAmuleto = 15;   // Custo em moedas de cada amuleto na Vila
    int faseAtual = 0;
    int proximaFase = 1;
    bool emFase = false;

    Mapa mapaAtual = CarregarMapa("bin/Debug/vila.txt");
    InicializarPosicaoJogador(mapaAtual, &jogador);

    Inimigo inimigos[MAX_INIMIGOS];
    SpawnarEntidadesDoMapa(mapaAtual, inimigos, spriteInimigo, spriteBoss);

    Camera2D camera = { 0 };
    camera.target = (Vector2)
    {
        jogador.x, jogador.y
    };
    camera.offset = (Vector2)
    {
        600.0f, 400.0f
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
            // Se o cheat já foi ativado antes, mantém a vida sempre cheia
            if (cheatVidaInfinitaAtivo)
            {
                jogador.vida.atual = jogador.vida.maxima;
            }

            // Detecta o clique na tecla Y
            if (IsKeyPressed(KEY_Y))
            {
                cliquesTeclaY++;
                if (cliquesTeclaY >= 3 && !cheatVidaInfinitaAtivo)
                {
                    cheatVidaInfinitaAtivo = true;
                    exibindoTelaCheat = true; // Ativa o aviso visual
                    estado = PAUSE;           // Pausa o jogo temporariamente para exibir a tela

                    PlaySound(somCheat);
                }
            }

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
// ALTERAÇÃO AQUI: Passando as variáveis corretas do main para a função

            // CORREÇÃO: Usar a constante global MAPA_COLUNAS em vez de mapaAtual.colunas
            AtualizarInimigos(inimigos, MAX_INIMIGOS, 0, MAPA_COLUNAS * TAMANHO_BLOCO, &jogador, mapaAtual);

            // 1. Sistema de CURA
            AtualizarCura(&jogador);

            // 2. Sistema de DASH
            AtualizarDash(&jogador);

            // 3. Aplicar Amuletos
            AplicarAmuletos(&jogador);

            // 4. Controle do Dash (tecla D)
            if (IsKeyPressed(KEY_D))
            {
                AtivarDash(&jogador);
            }

            if (IsKeyPressed(KEY_UP))
            {
                if (noChao)
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

            // --- COLETA DE AMULETOS (caractere 'A' no mapa) ---
            for (int i = 0; i < MAPA_LINHAS; i++)
            {
                for (int j = 0; j < MAPA_COLUNAS; j++)
                {
                    if (mapaAtual.matriz[i][j] == 'A')
                    {
                        Rectangle rectAmuleto = { j * TAMANHO_BLOCO, i * TAMANHO_BLOCO, TAMANHO_BLOCO, TAMANHO_BLOCO };

                        if (CheckCollisionRecs(rectJogador, rectAmuleto))
                        {
                            int tipoAmuleto = GetRandomValue(0, 5);

                            switch(tipoAmuleto)
                            {
                            case 0:
                                jogador.possuiAmuletoVida = true;
                                jogador.amuletoVidaExtra = true;
                                break;
                            case 1:
                                jogador.possuiAmuletoDano = true;
                                jogador.amuletoDanoExtra = true;
                                break;
                            case 2:
                                jogador.possuiAmuletoCura = true;
                                jogador.amuletoCuraEficiente = true;
                                break;
                            case 3:
                                jogador.possuiAmuletoEnergia = true;
                                jogador.amuletoEnergiaInfinita = true;
                                break;
                            case 4:
                                jogador.possuiAmuletoInvencivel = true;
                                jogador.amuletoInvencibilidade = true;
                                jogador.tempoInvencibilidadeAmuleto = 300;
                                break;
                            case 5:
                                jogador.possuiAmuletoEspecial = true;
                                jogador.amuletoAtaqueEspecial = true;
                                break;
                            }

                            mapaAtual.matriz[i][j] = ' ';
                        }
                    }
                }
            }

            // --- ATAQUE ESPECIAL (Amuleto) ---
            if (IsKeyPressed(KEY_F) && jogador.amuletoAtaqueEspecial &&
                    jogador.energia >= 30 && !jogador.ataqueEspecialAtivo)
            {
                jogador.ataqueEspecialAtivo = true;
                jogador.tempoAtaqueEspecial = 30;
                jogador.energia -= 30;

                for (int i = 0; i < MAX_INIMIGOS; i++)
                {
                    if (inimigos[i].ativo)
                    {
                        float distX = abs(inimigos[i].x - jogador.x);
                        float distY = abs(inimigos[i].y - jogador.y);

                        if (distX < 200 && distY < 200)
                        {
                            inimigos[i].vida -= 3;

                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;
                                jogador.moedas += inimigos[i].eChefe ? 5 : 1;
                            }
                        }
                    }
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
                                MudarDeFase(faseAtual, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss);
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
                            if (jogador.energia > jogador.energiaMaxima)
                            {
                                jogador.energia = jogador.energiaMaxima;
                            }

                            if (inimigos[i].vida <= 0)
                            {
                                inimigos[i].ativo = false;
                                jogador.moedas += inimigos[i].eChefe ? 5 : 1;

                                if (inimigos[i].eChefe)
                                {
                                    // Se tem próxima fase, avança o contador
                                    if (proximaFase < 2)
                                    {
                                        proximaFase++;
                                    }
                                    else
                                    {
                                        printf("PARABENS! VOCE COMPLETOU O JOGO!\n");
                                    }

                                    faseAtual = 0; // Prepara para voltar para a Vila
                                    emFase = false;
                                    jogador.vida.atual = jogador.vida.maxima;

                                    // Carrega efetivamente a Vila e tira o jogador da arena!
                                    MudarDeFase(faseAtual, &mapaAtual, &jogador, inimigos, spriteInimigo, spriteBoss);
                                }
                            } // Fecha if vida <= 0
                        } // Fecha if colisão
                    } // Fecha if ativo
                } // Fecha loop FOR
            } // Fecha if do ataque

            // Duração do Ataque
            if (jogador.atacando)
            {
                jogador.tempoAtaque--;
                if (jogador.tempoAtaque <= 0)
                {
                    jogador.atacando = false;
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
                        int dano = inimigos[i].eChefe ? 2 : 1;
                        ReceberDano(&jogador, dano);
                        jogador.tempoInvencivel = 60;

                        PlaySound(somDano);

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


            if (IsKeyPressed(KEY_I)) // <--- ADICIONE ESTE BLOCO
            {
                estado = INVENTARIO;
            }

        } // <--- FECHA O if(estado == JOGO) AQUI!

        // ==========================================
        // LÓGICA DO INVENTÁRIO (Pausa o Jogo)
        // ==========================================
        if (estado == INVENTARIO)
        {
            // Voltar para o jogo com ESC
            if (IsKeyPressed(KEY_ESCAPE))
            {
                estado = JOGO;
            }

            // Navegação entre os 3 blocos (esquerda e direita)
            if (IsKeyPressed(KEY_RIGHT))
            {
                slotSelecionado++;
                if (slotSelecionado > 2) slotSelecionado = 0;
            }
            if (IsKeyPressed(KEY_LEFT))
            {
                slotSelecionado--;
                if (slotSelecionado < 0) slotSelecionado = 2;
            }

            // Selecionar ou Comprar com ENTER
            if (IsKeyPressed(KEY_ENTER))
            {
                // Reseta os amuletos ativos primeiro (Regra: apenas um por vez)
                jogador.amuletoVidaExtra = false;
                jogador.amuletoDanoExtra = false;
                jogador.amuletoCuraEficiente = false;

                if (slotSelecionado == 0) // Slot 1: Vida
                {
                    if (jogador.possuiAmuletoVida)
                    {
                        jogador.amuletoVidaExtra = true;
                    }
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoVida = true;
                        jogador.amuletoVidaExtra = true;
                    }
                }
                else if (slotSelecionado == 1) // Slot 2: Dano
                {
                    if (jogador.possuiAmuletoDano)
                    {
                        jogador.amuletoDanoExtra = true;
                    }
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoDano = true;
                        jogador.amuletoDanoExtra = true;
                    }
                }
                else if (slotSelecionado == 2) // Slot 3: Cura
                {
                    if (jogador.possuiAmuletoCura)
                    {
                        jogador.amuletoCuraEficiente = true;
                    }
                    else if (faseAtual == 0 && jogador.moedas >= precoAmuleto)
                    {
                        jogador.moedas -= precoAmuleto;
                        jogador.possuiAmuletoCura = true;
                        jogador.amuletoCuraEficiente = true;
                    }
                }

                // Reaplica instantaneamente os buffs modificados (Limpo e Corrigido!)
                AplicarAmuletos(&jogador);
            }
        }


        // ==========================================
        // 2. DESENHO DO JOGO (Renderização na tela)
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
                if (!CarregarJogo(&jogador, &faseAtual))
                {
                    NovoJogo(&jogador, &faseAtual);
                }
                InicializarPosicaoJogador(mapaAtual, &jogador);
                estado = JOGO;
            }

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

            ClearBackground(BLACK);

            Rectangle sourceRecFundo = { 0.0f, 0.0f, (float)fundo.width, (float)fundo.height };
            Rectangle destRecFundo = { 0.0f, 0.0f, 1200.0f, 800.0f };
            Vector2 originFundo = { 0.0f, 0.0f };
            float rotacaoFundo = 0.0f;

            // Desenha o fundo usando as novas variáveis
            // Desenha o fundo usando as novas variáveis
            DrawTexturePro(fundo, sourceRecFundo, destRecFundo, originFundo, rotacaoFundo, WHITE);

            // 3. Entra no modo da câmera
            BeginMode2D(camera);
            DesenharMapa(mapaAtual);
            DesenharInimigos(inimigos, MAX_INIMIGOS);

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
                    destRecAtaque = (Rectangle)
                    {
                        jogador.x + jogador.largura, jogador.y + 10, 40, 40
                    };
                    DrawTexturePro(spriteAtaque, sourceRecAtaque, destRecAtaque, (Vector2)
                    {
                        0, 0
                    }, 0.0f, WHITE);
                }
                else
                {
                    sourceRecAtaque.width = -sourceRecAtaque.width;
                    destRecAtaque = (Rectangle)
                    {
                        jogador.x - 40, jogador.y + 10, 40, 40
                    };
                    DrawTexturePro(spriteAtaque, sourceRecAtaque, destRecAtaque, (Vector2)
                    {
                        0, 0
                    }, 0.0f, WHITE);
                }
            }

            if (pertoDoPoco)
            {
                DrawText("Aperte CIMA para entrar/sair", jogador.x - 60, jogador.y - 30, 15, WHITE);
            }

            EndMode2D();

            DesenharHUD(jogador);

            DrawText("S - Atacar", 20, 760, 20, RED);
            DrawText("ESC - Pause", 180, 760, 20, WHITE);
            DrawText("D - Dash", 360, 760, 20, BLUE);
            DrawText("A (segure) - Curar", 500, 760, 20, GREEN);
            DrawText("Seta p/ cima - Jump/Double Jump", 780, 760, 20, YELLOW);

            char textoFase[50];
            sprintf(textoFase, "Fase: %d/%d", proximaFase-1, 2);
            DrawText(textoFase, 1100 - MeasureText(textoFase, 20), 20, 20, GOLD);
        }
        else if (estado == PAUSE)
        {
            // SE FOR A TELA DO CHEAT CODE
            if (exibindoTelaCheat)
            {
                ClearBackground(BLACK); // Garante tela preta de fundo


                DrawTexture(imagemCheat, 450, 100, WHITE);
                DrawText("CHEAT CODE ATIVADO - NOOB DETECTADO", 250, 580, 35, RED);
                DrawText("clique em enter para continuar", 430, 640, 20, LIGHTGRAY);

                // Sai da tela de cheat e volta pro jogo ao apertar ENTER
                if (IsKeyPressed(KEY_ENTER))
                {
                    exibindoTelaCheat = false;
                    estado = JOGO;
                    cliquesTeclaY = 0; // Reseta o contador
                }
            }
            // SE FOR O PAUSE NORMAL DO ESC
            else
            {
                DrawTexture(imagemPause, 0, 0, WHITE);

                DrawText("GAME PAUSED", 460, 220, 40, MAROON);
                DrawText("Press C to Continue", 440, 340, 30, MAROON);
                DrawText("Press J to Save Game", 435, 400, 30, MAROON);
                DrawText("Press S to Exit to Menu", 410, 460, 30, MAROON);

                if (IsKeyPressed(KEY_C))
                    estado = JOGO;

                if (IsKeyPressed(KEY_J))
                {
                    SalvarJogo(&jogador, faseAtual);
                    DrawText("JOGO SALVO!", 300, 450, 30, GREEN);
                }

                if (IsKeyPressed(KEY_S))
                    estado = MENU;
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

            if (IsKeyPressed(KEY_ESCAPE))
            {
                estado = MENU;
            }
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

        else if (estado == INVENTARIO)
        {
            ClearBackground(BLACK); // Tela preta exigida

            DrawText("INVENTARIO DE AMULETOS", 400, 100, 30, WHITE);
            DrawText("Pressione ESC para fechar e voltar", 430, 150, 18, GRAY);
            DrawText(TextFormat("Suas Moedas: %d", jogador.moedas), 520, 200, 22, YELLOW);

            // Desenhar os 3 campos (Quadrados)
            for (int i = 0; i < 3; i++)
            {
                int posX = 250 + (i * 260);
                int posY = 320;

                // Se for o slot selecionado pelas setas, desenha borda amarela, se não, branca
                Color corBorda = (i == slotSelecionado) ? YELLOW : WHITE;
                DrawRectangleLines(posX, posY, 200, 200, corBorda);
                if (i == slotSelecionado)
                {
                    DrawRectangleLines(posX - 2, posY - 2, 204, 204, YELLOW); // Borda dupla de destaque
                }

                // Configura os textos específicos de cada slot
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

                // Exibe as informações dentro do quadrado
                DrawText(nome, posX + 15, posY + 20, 18, WHITE);
                DrawText(desc, posX + 15, posY + 60, 14, LIGHTGRAY);

                if (ativo)
                {
                    DrawText("EQUIPADO", posX + 50, posY + 140, 16, GREEN);
                }
                else if (possui)
                {
                    DrawText("POSSUI", posX + 65, posY + 140, 16, BLUE);
                }
                else
                {
                    if (faseAtual == 0)
                    {
                        DrawText("COMPRAR: 15$ [Enter]", posX + 15, posY + 140, 14, ORANGE);
                    }
                    else
                    {
                        DrawText("BLOQUEADO", posX + 50, posY + 140, 16, RED);
                    }
                }
            }

            // Mensagem de ajuda no rodapé
            if (faseAtual != 0)
            {
                DrawText("* Novos amuletos só podem ser comprados na Vila (Fase 0)", 350, 600, 16, RED);
            }
            else
            {
                DrawText("* Você está na Vila: Use Moedas para liberar itens trancados", 350, 600, 16, GREEN);
            }
        }

        EndDrawing();
    } // <--- FECHA O while

    // ==========================================
    // 3. FINALIZAÇÃO (FORA DO WHILE)
    // ==========================================
    UnloadTexture(spriteInimigo);
    UnloadTexture(spriteJogador);
    UnloadTexture(spriteAtaque);
    UnloadTexture(imagemPause);
    UnloadTexture(fundo);

    UnloadSound(somAtaque);
    UnloadSound(somDano);
    UnloadSound(somCheat);
    UnloadTexture(imagemCheat);

    CloseAudioDevice();

    CloseWindow();

    return 0;
} // <--- FECHA O main
