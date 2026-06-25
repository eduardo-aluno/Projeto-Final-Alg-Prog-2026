#ifndef JOGADOR_H
#define JOGADOR_H

#include "raylib.h"

typedef struct
{
    int atual;
    int maxima;
} Vida;

// --- Estrutura completa do Jogador ---
typedef struct Jogador
{
    // Posição e física
    float x;
    float y;
    float largura;
    float altura;
    float velocidadeY;
    bool noChao;

    // Combate
    bool atacando;
    int tempoAtaque;
    int tempoInvencivel;
    bool olhandoDireita;
    int danoAtaque;

    // Vida e recursos
    Vida vida;
    int moedas;
    int energia;
    int energiaMaxima;

    // Cura (segurar A)
    bool curando;
    int tempoCura;
    int curaDelay;
    int curaQuantidade;
    int custoCura;

    // Habilidades (Tecla D)
    bool dashDisponivel;
    bool dashAtivo;
    int tempoDash;
    float dashVelocidade;
    int custoDash;

    bool puloDuploDisponivel;
    bool puloDuploUsado;

    // Amuletos (sistema de buffs)
    bool amuletoVidaExtra;
    bool amuletoDanoExtra;
    bool amuletoCuraEficiente;
    bool amuletoEnergiaInfinita;
    bool amuletoInvencibilidade;
    bool amuletoAtaqueEspecial;

    // Inventário (quais amuletos o jogador possui)
    bool possuiAmuletoVida;
    bool possuiAmuletoDano;
    bool possuiAmuletoCura;
    bool possuiAmuletoEnergia;
    bool possuiAmuletoInvencivel;
    bool possuiAmuletoEspecial;

    // Controles para amuletos ativos
    int tempoInvencibilidadeAmuleto;
    bool ataqueEspecialAtivo;
    int tempoAtaqueEspecial;

    // NOVAS VARIÁVEIS PARA O ATAQUE ESPECIAL (DISPARO)
    bool projetilAtivo;
    float projetilX;
    float projetilY;
    float projetilVelocidadeX;
    int projetilDano;

    // Sprite
    Texture2D sprite;
} Jogador;

// --- Protótipos das funções ---
void DesenharHUD(Jogador jogador);
void ReceberDano(Jogador *jogador, int dano);
void InicializarJogador(Jogador *jogador);
void AtualizarCura(Jogador *jogador);
void AtivarDash(Jogador *jogador);
void AtualizarDash(Jogador *jogador);
void AplicarAmuletos(Jogador *jogador);
void AtualizarAmuletos(Jogador *jogador);

#endif
