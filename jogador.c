#include "jogador.h"
#include "raylib.h"
#include <stdio.h>

// --- Função para inicializar todos os campos do jogador ---
void InicializarJogador(Jogador *jogador)
{
    // Posição e física
    jogador->x = 0;
    jogador->y = 0;
    jogador->largura = 50;
    jogador->altura = 50;
    jogador->velocidadeY = 0;
    jogador->noChao = false;

    // Combate
    jogador->atacando = false;
    jogador->tempoAtaque = 0;
    jogador->tempoInvencivel = 0;
    jogador->olhandoDireita = true;
    jogador->danoAtaque = 1;

    // Vida e recursos
    jogador->vida.atual = 5;
    jogador->vida.maxima = 5;
    jogador->moedas = 0;
    jogador->energia = 0;
    jogador->energiaMaxima = 100;

    // Cura
    jogador->curando = false;
    jogador->tempoCura = 0;
    jogador->curaDelay = 30;
    jogador->curaQuantidade = 1;
    jogador->custoCura = 50;

    // Dash
    jogador->dashDisponivel = true;
    jogador->dashAtivo = false;
    jogador->tempoDash = 0;
    jogador->dashVelocidade = 15.0f;
    jogador->custoDash = 30;

    // Pulo duplo
    jogador->puloDuploDisponivel = false;
    jogador->puloDuploUsado = false;

    // Amuletos
    jogador->amuletoVidaExtra = false;
    jogador->amuletoDanoExtra = false;
    jogador->amuletoCuraEficiente = false;
    jogador->amuletoEnergiaInfinita = false;
    jogador->amuletoInvencibilidade = false;
    jogador->amuletoAtaqueEspecial = false;

    // Inventário
    jogador->possuiAmuletoVida = false;
    jogador->possuiAmuletoDano = false;
    jogador->possuiAmuletoCura = false;
    jogador->possuiAmuletoEnergia = false;
    jogador->possuiAmuletoInvencivel = false;
    jogador->possuiAmuletoEspecial = false;

    // Controles dos amuletos
    jogador->tempoInvencibilidadeAmuleto = 0;
    jogador->ataqueEspecialAtivo = false;
    jogador->tempoAtaqueEspecial = 0;

    // NOVAS VARIÁVEIS PARA ATAQUE ESPECIAL (CORRIGIDO)
    jogador->projetilAtivo = false;      // USANDO -> (ponteiro)
    jogador->projetilX = 0;              // USANDO ->
    jogador->projetilY = 0;              // USANDO ->
    jogador->projetilVelocidadeX = 0;    // USANDO ->
    jogador->projetilDano = 5;           // USANDO ->
}

// --- Função para atualizar a cura ---
void AtualizarCura(Jogador *jogador)
{
    if (IsKeyDown(KEY_A))
    {
        if (jogador->energia >= jogador->custoCura &&
            jogador->vida.atual < jogador->vida.maxima)
        {
            jogador->curando = true;
            jogador->tempoCura++;

            if (jogador->tempoCura >= jogador->curaDelay)
            {
                jogador->vida.atual += jogador->curaQuantidade;
                if (jogador->vida.atual > jogador->vida.maxima)
                    jogador->vida.atual = jogador->vida.maxima;
                jogador->energia -= jogador->custoCura;
                jogador->tempoCura = 0;
            }
        }
        else
        {
            jogador->curando = false;
            jogador->tempoCura = 0;
        }
    }
    else
    {
        jogador->curando = false;
        jogador->tempoCura = 0;
    }
}

// --- Função para ativar o dash ---
void AtivarDash(Jogador *jogador)
{
    if (jogador->dashDisponivel && !jogador->dashAtivo &&
        jogador->energia >= jogador->custoDash)
    {
        jogador->dashAtivo = true;
        jogador->tempoDash = 15;
        jogador->energia -= jogador->custoDash;
        jogador->dashDisponivel = false;
    }
}

// --- Função para atualizar o dash ---
void AtualizarDash(Jogador *jogador)
{
    if (jogador->dashAtivo)
    {
        jogador->tempoDash--;
        if (jogador->olhandoDireita)
            jogador->x += jogador->dashVelocidade;
        else
            jogador->x -= jogador->dashVelocidade;

        if (jogador->tempoDash <= 0)
        {
            jogador->dashAtivo = false;
            jogador->dashDisponivel = true;
        }
    }
}

// --- Função para aplicar os efeitos dos amuletos ---
void AplicarAmuletos(Jogador *jogador)
{
    // Reseta os efeitos base
    jogador->vida.maxima = 5;
    jogador->danoAtaque = 1;
    jogador->custoCura = 50;
    jogador->curaDelay = 30;

    // ===== AMULETO 1: VIDA EXTRA =====
    if (jogador->amuletoVidaExtra)
    {
        jogador->vida.maxima += 2;
    }

    // ===== AMULETO 2: DANO EXTRA =====
    if (jogador->amuletoDanoExtra)
    {
        jogador->danoAtaque += 1;
    }

    // ===== AMULETO 3: CURA EFICIENTE =====
    if (jogador->amuletoCuraEficiente)
    {
        jogador->custoCura = 30;
        jogador->curaDelay = 20;
    }

    // ===== AMULETO 4: ENERGIA INFINITA =====
    if (jogador->amuletoEnergiaInfinita)
    {
        jogador->energia = jogador->energiaMaxima;
        if (jogador->energia > jogador->energiaMaxima)
            jogador->energia = jogador->energiaMaxima;
    }

    // ===== AMULETO 5: INVENCIBILIDADE =====
    if (jogador->amuletoInvencibilidade)
    {
        if (jogador->tempoInvencibilidadeAmuleto <= 0)
        {
            jogador->tempoInvencibilidadeAmuleto = 300;
        }
    }

    // ===== AMULETO 6: ATAQUE ESPECIAL =====
    // (será controlado pelo main.c)

    // Se a vida atual ultrapassou a máxima, ajusta
    if (jogador->vida.atual > jogador->vida.maxima)
    {
        jogador->vida.atual = jogador->vida.maxima;
    }
}

// --- Função para receber dano (CORRIGIDA) ---
void ReceberDano(Jogador *jogador, int dano)
{
    // Verifica se está invencível por tempo ou pelo amuleto
    if (jogador->tempoInvencivel == 0 && jogador->tempoInvencibilidadeAmuleto == 0)
    {
        jogador->vida.atual -= dano;
        jogador->tempoInvencivel = 60; // 1 segundo de invencibilidade

        if (jogador->vida.atual < 0)
            jogador->vida.atual = 0;
    }
    // Se está invencível pelo amuleto, não toma dano
    else if (jogador->tempoInvencibilidadeAmuleto > 0)
    {
        // Mostra feedback visual (opcional)
        // DrawText("INVENCIBLE!", 400, 300, 30, GOLD);
        // Consome o tempo do amuleto
        // O tempo já está sendo consumido em AtualizarAmuletos()
    }
    // Se está invencível por tempo normal, não faz nada
    // else {}
} // <-- CHAVE FECHADA CORRETAMENTE!

// --- Função para desenhar o HUD ---
void DesenharHUD(Jogador jogador)
{
    // Fundo do HUD
    DrawRectangle(10, 10, 300, 80, Fade(BLACK, 0.5f));

    // Vidas (corações)
    DrawText("Vida: ", 20, 20, 20, WHITE);
    for (int i = 0; i < jogador.vida.maxima; i++)
    {
        Color cor = (i < jogador.vida.atual) ? RED : GRAY;
        DrawText("*", 80 + (i * 25), 18, 25, cor);
    }

    // Moedas
    DrawText(TextFormat("Moedas: %d", jogador.moedas), 20, 45, 20, YELLOW);

    // Energia (barra)
    DrawText("Energia:", 20, 70, 15, WHITE);
    float porcentagemEnergia = (float)jogador.energia / jogador.energiaMaxima;
    DrawRectangle(90, 70, 150, 15, DARKGRAY);
    DrawRectangle(90, 70, 150 * porcentagemEnergia, 15, BLUE);
    DrawRectangleLines(90, 70, 150, 15, WHITE);
    DrawText(TextFormat("%d/%d", jogador.energia, jogador.energiaMaxima),
             245, 70, 15, WHITE);

    // Indicador de cura
    if (jogador.curando)
    {
        DrawText("CURANDO...", 20, 95, 15, GREEN);
        float progresso = (float)jogador.tempoCura / jogador.curaDelay;
        DrawRectangle(20, 115, 100 * progresso, 8, GREEN);
        DrawRectangleLines(20, 115, 100, 8, WHITE);
    }

    // Indicador de dash
    if (!jogador.dashDisponivel)
    {
        DrawText("Dash: Recarregando", 20, 130, 12, GRAY);
    }

    // Mostrar amuletos equipados
    int yOffset = 150;
    if (jogador.amuletoVidaExtra)
    {
        DrawText("+ Amuleto Vida", 20, yOffset, 12, PINK);
        yOffset += 20;
    }
    if (jogador.amuletoDanoExtra)
    {
        DrawText("+ Amuleto Dano", 20, yOffset, 12, PINK);
        yOffset += 20;
    }
    if (jogador.amuletoCuraEficiente)
    {
        DrawText("+ Amuleto Cura", 20, yOffset, 12, PINK);
        yOffset += 20;
    }
    if (jogador.amuletoEnergiaInfinita)
    {
        DrawText("+ Energia Infinita", 20, yOffset, 12, RED);
        yOffset += 20;
    }
    if (jogador.amuletoInvencibilidade)
    {
        DrawText("+ Invencibilidade", 20, yOffset, 12, GOLD);
        yOffset += 20;
    }
    if (jogador.amuletoAtaqueEspecial)
    {
        DrawText("+ Ataque Especial (F)", 20, yOffset, 12, ORANGE);
        yOffset += 20;
    }
}

// --- Função para atualizar os amuletos a cada frame ---
void AtualizarAmuletos(Jogador *jogador)
{
    // Amuleto de Invencibilidade
    if (jogador->amuletoInvencibilidade)
    {
        if (jogador->tempoInvencibilidadeAmuleto > 0)
        {
            jogador->tempoInvencibilidadeAmuleto--;
        }
    }

    // Amuleto de Energia Infinita
    if (jogador->amuletoEnergiaInfinita)
    {
        if (jogador->energia < jogador->energiaMaxima)
        {
            jogador->energia = jogador->energiaMaxima;
        }
    }

    // Amuleto de Ataque Especial
    if (jogador->ataqueEspecialAtivo)
    {
        jogador->tempoAtaqueEspecial--;
        if (jogador->tempoAtaqueEspecial <= 0)
        {
            jogador->ataqueEspecialAtivo = false;
        }
    }
}
