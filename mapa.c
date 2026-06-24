#include "mapa.h"
#include "jogador.h"
#include "raylib.h"
#include <string.h>

static Texture2D texturaChaoGlobal;

void SetTexturaChao(Texture2D textura) {
    texturaChaoGlobal = textura;
}

Mapa CarregarMapa(const char *caminhoArquivo)
{
    Mapa mapa;
    FILE *arquivo = fopen(caminhoArquivo, "r");

    if (arquivo == NULL)
    {
        printf("ERRO: Nao consegui abrir o arquivo %s!\n", caminhoArquivo);
        // Inicializa matriz com espaços em caso de erro
        for (int l = 0; l < MAPA_LINHAS; l++)
            for (int c = 0; c < MAPA_COLUNAS; c++)
                mapa.matriz[l][c] = ' ';
        return mapa;
    }

    printf("Arquivo %s aberto com sucesso!\n", caminhoArquivo);

    // Leitura linha por linha (mais confiável)
    for (int l = 0; l < MAPA_LINHAS; l++)
    {
        char linha[MAPA_COLUNAS + 10]; // +10 para segurança
        if (fgets(linha, sizeof(linha), arquivo) != NULL)
        {
            // Remove a quebra de linha do final, se existir
            int len = strlen(linha);
            if (len > 0 && (linha[len-1] == '\n' || linha[len-1] == '\r'))
                linha[len-1] = '\0';
            if (len > 1 && linha[len-2] == '\r') // Para arquivos Windows
                linha[len-2] = '\0';

            // Copia os caracteres para a matriz
            for (int c = 0; c < MAPA_COLUNAS; c++)
            {
                if (c < (int)strlen(linha) && linha[c] != '\0')
                    mapa.matriz[l][c] = linha[c];
                else
                    mapa.matriz[l][c] = ' '; // Preenche com espaço se linha for curta
            }
        }
        else
        {
            // Se não conseguiu ler a linha, preenche com espaços
            for (int c = 0; c < MAPA_COLUNAS; c++)
                mapa.matriz[l][c] = ' ';
        }
    }

    fclose(arquivo);

    // Debug: print dos primeiros caracteres
    printf("Primeira linha do mapa: ");
    for (int c = 0; c < 10; c++)
        printf("%c", mapa.matriz[0][c]);
    printf("\n");

    return mapa;
}

void DesenharMapa(Mapa mapa)
{
    for (int l = 0; l < MAPA_LINHAS; l++)
    {
        for (int c = 0; c < MAPA_COLUNAS; c++)
        {
            // Calcula a posição real de renderização baseada nos índices da matriz
            float posX = c * TAMANHO_BLOCO;
            float posY = l * TAMANHO_BLOCO;

            // Filtra o caractere correspondente
            switch (mapa.matriz[l][c])
            {
case 'P': // Paredes/Chão estrutural
    // ===== USA A TEXTURA SE ESTIVER CARREGADA =====
    if (texturaChaoGlobal.id != 0) {
        Rectangle sourceRec = { 0.0f, 0.0f,
                               (float)texturaChaoGlobal.width,
                               (float)texturaChaoGlobal.height };
        Rectangle destRec = { posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO };
        DrawTexturePro(texturaChaoGlobal, sourceRec, destRec,
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Fallback: bloco cinza se a textura não foi carregada
        DrawRectangle(posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO, DARKGRAY);
        DrawRectangleLines(posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO, GRAY);
    }
    break;
            case 'p': // Plataformas internas ou paredes menores
                DrawRectangle(posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO, LIGHTGRAY);
                break;
            case 'H': // Habilidade
                DrawPoly((Vector2)
                {
                    posX + TAMANHO_BLOCO/2, posY + TAMANHO_BLOCO/2
                }, 3, 12, 0, PURPLE);
                break;
            case 'E': // Entrada/Poço para a fase
                DrawRectangle(posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO, BROWN);
                DrawRectangleLines(posX, posY, TAMANHO_BLOCO, TAMANHO_BLOCO, DARKBROWN);
                break;
            default:
                // Espaços vazios (' '), monstros ('M'), jogador ('J') ou boss ('C')
                // não desenhamos aqui, pois eles serão entidades móveis separadas.
                break;
            case 'A': // Amuleto
                // Desenha um diamante brilhante
                DrawPoly((Vector2)
                {
                    posX + TAMANHO_BLOCO/2, posY + TAMANHO_BLOCO/2
                },
                4, 15, 45, GOLD);
                DrawPoly((Vector2)
                {
                    posX + TAMANHO_BLOCO/2, posY + TAMANHO_BLOCO/2
                },
                4, 10, 45, YELLOW);
                // Brilho
                DrawCircle(posX + TAMANHO_BLOCO/2, posY + TAMANHO_BLOCO/2,
                           5, (Color)
                {
                    255, 255, 255, 100
                });
                break;
            }
        }
    }
}

void InicializarPosicaoJogador(Mapa mapa, Jogador *jogador)
{
    for (int l = 0; l < MAPA_LINHAS; l++)
    {
        for (int c = 0; c < MAPA_COLUNAS; c++)
        {
            if (mapa.matriz[l][c] == 'J')
            {
                jogador->x = c * TAMANHO_BLOCO;
                // Ajusta a altura baseado no bloco de 50 pixels
                jogador->y = l * TAMANHO_BLOCO;
                return;
            }
        }
    }
    jogador->x = 100;
    jogador->y = 200;
}
