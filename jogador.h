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
    bool noChao;              // NOVO: Para saber se está no chão

    // Combate
    bool atacando;
    int tempoAtaque;
    int tempoInvencivel;
    bool olhandoDireita;
    int danoAtaque;           // NOVO: Dano do ataque (pode ser modificado por amuletos)

    // Vida e recursos
    Vida vida;
    int moedas;
    int energia;
    int energiaMaxima;

    // Cura (segurar A)
    bool curando;             // NOVO: Se está ativamente curando
    int tempoCura;            // NOVO: Tempo que está segurando A
    int curaDelay;            // NOVO: Frames entre cada cura (30 = 0.5s)
    int curaQuantidade;       // NOVO: Quantidade curada por vez (padrão 1)
    int custoCura;            // NOVO: Custo em energia para curar (padrão 50)

    // Habilidades (Tecla D)
    bool dashDisponivel;      // NOVO: Se o dash está disponível
    bool dashAtivo;           // NOVO: Se está realizando o dash
    int tempoDash;            // NOVO: Duração do dash em frames
    float dashVelocidade;     // NOVO: Velocidade do dash
    int custoDash;            // NOVO: Custo em energia do dash

    bool puloDuploDisponivel; // NOVO: Se o pulo duplo foi desbloqueado
    bool puloDuploUsado;      // NOVO: Se o pulo duplo já foi usado no salto atual

    // Amuletos (sistema de buffs)
    bool amuletoVidaExtra;         // +2 vidas
    bool amuletoDanoExtra;         // +1 dano
    bool amuletoCuraEficiente;     // Cura mais barata
    bool amuletoEnergiaInfinita;   // Enche 100% da energia
    bool amuletoInvencibilidade;   // Invencível por tempo limitado
    bool amuletoAtaqueEspecial;    // Ataque especial

// Inventário (quais amuletos o jogador possui)
    bool possuiAmuletoVida;
    bool possuiAmuletoDano;
    bool possuiAmuletoCura;
    bool possuiAmuletoEnergia;
    bool possuiAmuletoInvencivel;
    bool possuiAmuletoEspecial;

    // Controles para amuletos ativos
    int tempoInvencibilidadeAmuleto;  // NOVO: Contador de invencibilidade do amuleto
    bool ataqueEspecialAtivo;         // NOVO: Se o ataque especial está ativo
    int tempoAtaqueEspecial;          // NOVO: Duração do ataque especial

    // Sprite
    Texture2D sprite;
} Jogador;

// --- Protótipos das funções ---
void DesenharHUD(Jogador jogador);
void ReceberDano(Jogador *jogador, int dano);

// NOVAS FUNÇÕES PARA O SISTEMA DE CURA E HABILIDADES
void InicializarJogador(Jogador *jogador);  // Função para inicializar todos os campos
void AtualizarCura(Jogador *jogador);       // Atualiza o estado de cura
void AtivarDash(Jogador *jogador);          // Ativa o dash
void AtualizarDash(Jogador *jogador);       // Atualiza o estado do dash
void AplicarAmuletos(Jogador *jogador);     // Aplica os efeitos dos amuletos

#endif
