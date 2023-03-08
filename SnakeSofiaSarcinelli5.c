#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { // POSICAO NA MATRIZ
    int y;
    int x;
} tPosicao;

typedef struct { // CARACTERISTICAS DE CADA JOGADA
    char movimento;
    tPosicao posProxCelula;
    char caractereProxCelula;
    int qtdMovimentos;

    // CONTAGEM PARA ESTATISTICAS
    int qtdMovSemPontuar;
    int qtdMovCima;
    int qtdMovBaixo;
    int qtdMovEsq;
    int qtdMovDir;
} tJogada;

typedef struct { // HEATMAP ESPECIFICAMENTE
    FILE * arquivo;
    int matriz[110][110];
} tHeatMap;

typedef struct { // INFORMACOES DO MAPA
    FILE * arquivoEntrada; // ARQUIVO MAPA.TXT
    FILE * arquivoSaida; //ARQUIVO INICIALIZACAO.TXT
    int linhas;
    int colunas;
    char matriz[110][110];
    char dirCabecaAnterior; // CHAR DA CABECA ANTERIOR: 'v', '>', '^', ou '<'

    // COBRA
    tPosicao cabeca;
    tPosicao cabecaAntiga;
    tPosicao cobra[110];
    tPosicao rabo;
    tPosicao raboAntigo;
    int tamanhoCobra;
} tMapa;

typedef struct { // INFORMACOES GERAIS DO JOGO
    int pontuacao;
    int flagEhFim; // TRUE SE JOGO ACABOU E FALSE CASO CONTRARIO
    int flagEhRabo; // TRUE SE A COBRA FOR PARA A CELULA ONDE SEU RABO ESTA E FALSE CASO CONTRARIO
    tMapa mapa;
    tJogada jogada;
    tHeatMap heatMap;
    char arg1[1001];
} tJogo;


#define VAZIO ' '
#define PAREDE '#'
#define COMIDA '*'
#define DINHEIRO '$'
#define CABECADIREITA '>'
#define CABECAESQUERDA '<'
#define CABECACIMA '^'
#define CABECABAIXO 'v'
#define CORPO 'o'
#define CONTINUE 'c'
#define ANTIHORARIO 'a'
#define DIRETORIO 1500
#define HORARIO 'h'
#define PONTOSCOMIDA 1
#define PONTOSDINHEIRO 10
#define VENCEU 1
#define PERDEU 2
#define CORPINHOPORCOMIDA 1
#define CARCACA 'X'

//FUNCOES GERAIS DO JOGO
tJogo AbreArquivos(tJogo jogo, char arg1[]);
tJogo FechaArquivos(tJogo jogo);
tJogo InicializaJogo(tJogo jogo);
tJogo RealizaJogada(tJogo jogo);
tJogada LeMovimento(tJogada jogada);
void PrintaEstadoDoJogo(tJogo jogo);
void FinalizaJogo(tJogo jogo);

//ACOES JOGO
tJogo FazAcaoComida(tJogo jogo);
tJogo FazAcaoDinheiro(tJogo jogo);
tJogo FazAcoesDeColisoes(tJogo jogo);

// FUNCOES DE MAPA
tJogo AndaCucaCobra(tJogo jogo);
tMapa TransformaCorpoEmVazio(tMapa mapa, tPosicao posicao);
tMapa AtualizaCorpoCobra(tMapa mapa);
tMapa CriaMatrizMapa(tMapa mapa);
tMapa PrintaMatrizEmArquivo(tMapa mapa);
tMapa AumentaCobra(tMapa mapa);
tMapa CobraViraX(tMapa mapa);
void PrintaMatrizNoTerminal(tMapa mapa);
tPosicao AtravessaLados(tMapa mapa, tJogada jogada);

// BOOLEANAS
int EhCabeca(char x);
int RetornaQtdLinhasMapa(tMapa mapa);
int RetornaQtdColunasMapa(tMapa mapa);
int EhFimDoJogo(tJogo jogo);
int TemComidaAinda(tMapa mapa);
int PosicoesSaoDiferentes(tPosicao posicao1, tPosicao posicao2);
int CoordenadaEstaForaDaMatriz(tMapa mapa, tPosicao celula);

// ARQUIVOS
void AbreResumo(char arg1[]);
void CriaResumo(tJogo jogo, char arg1[]);
void GeraRanking(tJogo jogo);
void GeraEstatisticas(tJogo jogo);
tHeatMap CriaMatrizHeatMap(tMapa mapa);
void PrintaMatrizHeat(tJogo jogo);

// OUTRAS
char QualCaractereEstaNaCelula(tMapa mapa, tPosicao posicao);
tPosicao ChecaPosProxCelula(tMapa mapa, tJogada jogada);

int EhCabeca(char x) { // CHECA SE CARACTERE QUALQUER E CABECA DA COBRA OU NAO

    return ((x == CABECADIREITA) || (x == CABECAESQUERDA) || (x == CABECACIMA) || (x == CABECABAIXO));

}

tMapa CriaMatrizMapa(tMapa mapa) { // RECEBE A VARIAVEL MAPA E CRIA A MATRIZ
    int i = 0, j = 0;
    fscanf(mapa.arquivoEntrada, "%d %d%*c", &mapa.linhas, &mapa.colunas);
    mapa.colunas++;
    for (i = 0; i < mapa.linhas; i++) {
        for (j = 0; j < mapa.colunas; j++) {
            fscanf(mapa.arquivoEntrada, "%c", &mapa.matriz[i][j]);
            if (EhCabeca(mapa.matriz[i][j])) {
                mapa.cabeca.y = i;
                mapa.cabeca.x = j;
            }
        }
    }
    return mapa;
}

int RetornaQtdLinhasMapa(tMapa mapa) {
    return mapa.linhas;
}

int RetornaQtdColunasMapa(tMapa mapa) {
    return mapa.colunas;
}

void PrintaMatrizNoTerminal(tMapa mapa) { // RECEBE MAPA E PRINTA MATRIZ NO TERMINAL
    int i = 0, j = 0;

    for (i = 0; i < mapa.linhas; i++) {
        for (j = 0; j < mapa.colunas; j++) {
            if (!(i == mapa.linhas - 1 && j == mapa.colunas - 1)) {
                printf("%c", mapa.matriz[i][j]);
            }
        }
    }
}

tMapa PrintaMatrizEmArquivo(tMapa mapa) { // RECEBE MAPA E PRINTA MATRIZ NO ARQUIVO INICIALIZACAO.TXT
    int i = 0, j = 0;

    for (i = 0; i < mapa.linhas; i++) {
        for (j = 0; j < mapa.colunas; j++) {
            if (!(i == mapa.linhas - 1 && j == mapa.colunas - 1)) {
                fprintf(mapa.arquivoSaida, "%c", mapa.matriz[i][j]);
            }
        }
    }
    return mapa;
}

tJogo AbreArquivos(tJogo jogo, char arg1[]) { // ABRE ARQUIVOS UTILIZADOS FUTURAMENTE
    char diretorio[DIRETORIO], enter = 's';

    // ABRE ARQUIVO MAPA.TXT
    sprintf(diretorio, "%s/mapa.txt", arg1);
    jogo.mapa.arquivoEntrada = fopen(diretorio, "r");

    if (!jogo.mapa.arquivoEntrada) {
        printf("Nao foi possivel ler o arquivo %s\n", diretorio);
        exit(0);
    }

    // ABRE ARQUIVO INICIALIZACAO.TXT
    sprintf(diretorio, "%s/saida/inicializacao.txt", arg1);
    jogo.mapa.arquivoSaida = fopen(diretorio, "w");

    return jogo;
}

tJogo FechaArquivos(tJogo jogo) { // FECHA ARQUIVOS USADOS

    fclose(jogo.mapa.arquivoEntrada);
    fclose(jogo.mapa.arquivoSaida);

    return jogo;
}

tHeatMap CriaMatrizHeatMap(tMapa mapa) { // CRIA MATRIZ COM MESMAS DIMENSOES DO MAPA MAS COM ZEROS NO LUGAR DE CARACTERES
    tHeatMap heatMap;
    int contLinhas = 0, contColunas = 0;

    for (contLinhas = 0; contLinhas < mapa.linhas; contLinhas++) {
        for (contColunas = 0; contColunas < mapa.colunas - 1; contColunas++) {
            heatMap.matriz[contLinhas][contColunas] = 0;
        }
    }
    return heatMap;
}

void PrintaMatrizHeat(tJogo jogo) { // CRIA ARQUIVO E PRINTA HEATMAP NELE
    int contLinhas = 0, contColunas = 0;
    FILE * arquivo;
    char diretorio[DIRETORIO];

    // ABRE ARQUIVO HEATMAP.TXT
    sprintf(diretorio, "%s/saida/heatmap.txt", jogo.arg1); //ABRE ARQUIVO HEATMAP.TXT
    arquivo = fopen(diretorio, "w");

    // PRINTA HEATMAP
    for (contLinhas = 0; contLinhas < jogo.mapa.linhas; contLinhas++) { // VARRE A MATRIZ E PRINTA NO ARQUIVO
        for (contColunas = 0; contColunas < jogo.mapa.colunas - 1; contColunas++) {
            if (contColunas != 0) {
                fprintf(arquivo, " ");
            }
            fprintf(arquivo, "%d", jogo.heatMap.matriz[contLinhas][contColunas]);
        }
        fprintf(arquivo, "\n");
    }

    fclose(arquivo); // FECHA ARQUIVO HEATMAP.TXT
}

void AbreResumo(char arg1[]) { // ABRE ARQUIVO RESUMO.TXT
    FILE * arquivo;
    char diretorio[DIRETORIO];
    sprintf(diretorio, "%s/saida/resumo.txt", arg1);
    arquivo = fopen(diretorio, "w");
}

tJogo InicializaJogo(tJogo jogo) { // CRIA ARQUIVO INICIALIZACAO.TXT COM MATRIZ E POSICAO INICIAL DA CABECA DA COBRA

    // ABRE ARQUIVOS
    jogo = AbreArquivos(jogo, jogo.arg1);
    AbreResumo(jogo.arg1);

    // LE E ARMAZENA MAPA EM MATRIZ JOGO.MAPA
    jogo.mapa = CriaMatrizMapa(jogo.mapa);

    //PRINTA MATRIZ NO ARQUIVO INICIALIZACAO.TXT
    jogo.mapa = PrintaMatrizEmArquivo(jogo.mapa);

    // ESTABELECE INFORMACOES INICIAIS DA COBRA
    jogo.mapa.cobra[0] = jogo.mapa.cabeca;
    jogo.mapa.rabo = jogo.mapa.cabeca;
    jogo.mapa.tamanhoCobra = 1;

    // PRINTA POS INICIAL DA COBRA NO ARQUIVO INICIALIZACAO.TXT
    fprintf(jogo.mapa.arquivoSaida, "\nA cobra comecara o jogo na linha %d e coluna %d\n", jogo.mapa.cabeca.y + 1, jogo.mapa.cabeca.x + 1);

    // CRIA MATRIZ HEATMAP ZERADA
    jogo.heatMap = CriaMatrizHeatMap(jogo.mapa);

    // ATUALIZA MATRIZ HEATMAP COM A POSICAO INICIAL DA CABECA
    jogo.heatMap.matriz[jogo.mapa.cabeca.y][jogo.mapa.cabeca.x] += 1;

    return jogo;

}

tJogada LeMovimento(tJogada jogada) { //SCANF DE CADA MOVIMENTO
    char movimento, enter;
    scanf("%c%c", &jogada.movimento, &enter);
    return jogada;
}

tMapa TransformaCorpoEmVazio(tMapa mapa, tPosicao posicao) {
    mapa.matriz[posicao.y][posicao.x] = ' ';
    return mapa;
}

tMapa AtualizaCorpoCobra(tMapa mapa) { // ANDA CORPO DA COBRA
    int i = 1;
    tPosicao posTemporaria[101]; // VETOR DE POSICOES AUXILIAR PARA GUARDAR A POSICAO ANTIGA DA COBRA

    posTemporaria[0] = mapa.cabecaAntiga;

    for (i = 1; i < mapa.tamanhoCobra; i++) { // CADA BOLINHA DO CORPO VAI PRA POSICAO ANTIGA DA BOLINHAS ANTERIOR
        posTemporaria[i] = mapa.cobra[i];
        mapa.cobra[i] = posTemporaria[i - 1];
        mapa.matriz[mapa.cobra[i].y][mapa.cobra[i].x] = CORPO;
        if (PosicoesSaoDiferentes(posTemporaria[i], mapa.cabeca)) { // SE A COBRA ANDAR ATRAS DO SEU PROPRIO RABO NAO APAGA A CABECA
            mapa = TransformaCorpoEmVazio(mapa, posTemporaria[i]);
        }
        mapa.rabo = mapa.cobra[i]; // ESTABELECE O RABO DA COBRA
    }
    if (mapa.tamanhoCobra == 1) { // SE A COBRA N TIVER CORPO ESTABELECE A CABECA COMO RABO
        mapa.raboAntigo = mapa.rabo;
        mapa.rabo = mapa.cabeca;
    }
    return mapa;
}

tPosicao ChecaPosProxCelula(tMapa mapa, tJogada jogada) { // DESCOBRE QUAL POSICAO NA MATRIZ A COBRA DEVE IR
    tPosicao proxCelula;
    proxCelula = mapa.cabeca;
    switch (jogada.movimento) { // JOGADA.MOVIMENTO PODE SER CONTINUAR, HORARIO OU ANTI-HORARIO
        case CONTINUE:
            switch (mapa.matriz[mapa.cabeca.y][mapa.cabeca.x]) {
                case CABECADIREITA:
                    proxCelula.x = mapa.cabeca.x + 1;
                    break;
                case CABECAESQUERDA:
                    proxCelula.x = mapa.cabeca.x - 1;
                    break;
                case CABECACIMA:
                    proxCelula.y = mapa.cabeca.y - 1;
                    break;
                case CABECABAIXO:
                    proxCelula.y = mapa.cabeca.y + 1;
                    break;
            }
            break;
        case HORARIO:
            switch (mapa.matriz[mapa.cabeca.y][mapa.cabeca.x]) {
                case CABECADIREITA:
                    proxCelula.y = mapa.cabeca.y + 1;
                    break;
                case CABECAESQUERDA:
                    proxCelula.y = mapa.cabeca.y - 1;
                    break;
                case CABECACIMA:
                    proxCelula.x = mapa.cabeca.x + 1;
                    break;
                case CABECABAIXO:
                    proxCelula.x = mapa.cabeca.x - 1;
                    break;
            }
            break;
        case ANTIHORARIO:
            switch (mapa.matriz[mapa.cabeca.y][mapa.cabeca.x]) {
                case CABECADIREITA:
                    proxCelula.y = mapa.cabeca.y - 1;
                    break;
                case CABECAESQUERDA:
                    proxCelula.y = mapa.cabeca.y + 1;
                    break;
                case CABECACIMA:
                    proxCelula.x = mapa.cabeca.x - 1;
                    break;
                case CABECABAIXO:
                    proxCelula.x = mapa.cabeca.x + 1;
                    break;
            }
            break;
    }
    return proxCelula;
}

char QualCaractereEstaNaCelula(tMapa mapa, tPosicao posicao) { // RETORNA QUAL CARACTERE ESTA NA POSICAO DADA DO MAPA
    char caractere;

    caractere = mapa.matriz[posicao.y][posicao.x];

    return caractere;

}

tMapa AumentaCobra(tMapa mapa) { // AUMENTA CORPO DA COBRA EM 1 

    mapa.cobra[mapa.tamanhoCobra - 1] = mapa.raboAntigo;
    mapa.matriz[mapa.raboAntigo.y][mapa.raboAntigo.x] = CORPO;
    mapa.rabo = mapa.raboAntigo;

    return mapa;
}

tJogo FazAcaoComida(tJogo jogo) { // GANHA PONTUACAO POR COMIDA E AUMENTA O CORPO
    jogo.pontuacao += PONTOSCOMIDA;

    jogo.mapa.tamanhoCobra += CORPINHOPORCOMIDA;
    jogo.mapa = AumentaCobra(jogo.mapa);

    return jogo;
}

tJogo FazAcaoDinheiro(tJogo jogo) { // AUMENTA PONTUACAO PELO DINHEIRO
    jogo.pontuacao += PONTOSDINHEIRO;
    return jogo;
}

tJogo FazAcoesDeColisoes(tJogo jogo) { // CUIDA DAS ACOES SE A COBRA COMER ALGO OU BATER

    switch (jogo.jogada.caractereProxCelula) { // DEPENDE DO CARACTERE NA POSICAO DA MATRIZ QUE A CABECA VAI
        case PAREDE:
            jogo.flagEhFim = PERDEU;
            jogo.jogada.qtdMovSemPontuar++;
            break;
        case COMIDA:
            jogo = FazAcaoComida(jogo);
            break;
        case DINHEIRO:
            jogo = FazAcaoDinheiro(jogo);
            break;
        case CORPO:
            if (PosicoesSaoDiferentes(jogo.mapa.raboAntigo, jogo.jogada.posProxCelula)) { // SE A CABECA ANDAR PRA ONDE ESTAVA O RABO ELA NAO PERDE
                jogo.flagEhFim = PERDEU;
            } else {
                jogo.flagEhRabo = 1;
            }
            jogo.jogada.qtdMovSemPontuar++;
            break;
        default:
            jogo.jogada.qtdMovSemPontuar++;
    }
    return jogo;
}

int CoordenadaEstaForaDaMatriz(tMapa mapa, tPosicao celula) { // RETORNA SE POSICAO ESTA FORA DA MATRIZ
    if (celula.x < 0 || celula.x > mapa.colunas - 2 || celula.y > mapa.linhas - 1 || celula.y < 0) {
        return 1;
    }
    return 0;
}

tPosicao AtravessaLados(tMapa mapa, tJogada jogada) { // FAZ A COBRA REAPARECER DO OUTRO LADO DO MADA

    // SE ELA SAIR PRA ESQUERDA
    if (jogada.posProxCelula.x < 0) {
        jogada.posProxCelula.x = mapa.colunas - 2;
    }// SE ELA SAIR PRA DIREITA
    else if (jogada.posProxCelula.x > mapa.colunas - 2) {
        jogada.posProxCelula.x = 0;
    }// SE ELA SAIR PRA BAIXO
    else if (jogada.posProxCelula.y > mapa.linhas - 1) {
        jogada.posProxCelula.y = 0;
    }// SE ELA SAIR PRA CIMA
    else if (jogada.posProxCelula.y < 0) {
        jogada.posProxCelula.y = mapa.linhas - 1;
    }
    return jogada.posProxCelula;
}

void CriaResumo(tJogo jogo, char arg1[]) { //CRIA RESUMO DOS MOVIMENTOS QUE AUMENTARAM PONTUACAO OU TERMINARAM O JOGO
    FILE * arquivo;
    char diretorio[DIRETORIO];

    sprintf(diretorio, "%s/saida/resumo.txt", arg1);
    // ABRE RESUMO.TXT
    arquivo = fopen(diretorio, "a");

    fprintf(arquivo, "Movimento %d (%c)", jogo.jogada.qtdMovimentos, jogo.jogada.movimento);

    switch (jogo.jogada.caractereProxCelula) { // DEPENDE DO CARACTERE QUE TEM NA POSICAO QUE A CABECA VAI
        case COMIDA:
            fprintf(arquivo, " fez a cobra crescer para o tamanho %d", jogo.mapa.tamanhoCobra);
            if (EhFimDoJogo(jogo)) {
                fprintf(arquivo, ", terminando o jogo\n");
            } else {
                fprintf(arquivo, "\n");
            }
            break;
        case DINHEIRO:
            fprintf(arquivo, " gerou dinheiro\n");
            break;
        default:
            if (EhFimDoJogo(jogo)) {
                fprintf(arquivo, " resultou no fim de jogo por conta de colisao\n");
            }
            break;
    }

    fclose(arquivo); // FECHA RESUMO.TXT 
}

void PrintaEstadoDoJogo(tJogo jogo) { // PRINTA MOVIMENTO, MAPA E PONTUACAO NO TERMINAL

    printf("\nEstado do jogo apos o movimento '%c':\n", jogo.jogada.movimento);
    PrintaMatrizNoTerminal(jogo.mapa);
    printf("\n");
    printf("Pontuacao: %d\n", jogo.pontuacao);
}

tJogo AndaCucaCobra(tJogo jogo) { // ANDA CABECA COBRA
    char cabecaNova = 's';
    switch (jogo.jogada.movimento) { // LEVA EM CONTA O MOVIMENTO FEITO ('h','a' ou 'c')
        case HORARIO:
            switch (jogo.mapa.dirCabecaAnterior) { // LEVA EM CONTA A DIRECAO DA CABECA ANTES
                case CABECADIREITA:
                    cabecaNova = 'v';
                    jogo.jogada.qtdMovBaixo++;
                    break;
                case CABECAESQUERDA:
                    cabecaNova = '^';
                    jogo.jogada.qtdMovCima++;
                    break;
                case CABECACIMA:
                    cabecaNova = '>';
                    jogo.jogada.qtdMovDir++;
                    break;
                case CABECABAIXO:
                    cabecaNova = '<';
                    jogo.jogada.qtdMovEsq++;
                    break;
            }
            break;
        case ANTIHORARIO:
            switch (jogo.mapa.dirCabecaAnterior) {
                case CABECADIREITA:
                    cabecaNova = '^';
                    jogo.jogada.qtdMovCima++;
                    break;
                case CABECAESQUERDA:
                    cabecaNova = 'v';
                    jogo.jogada.qtdMovBaixo++;
                    break;
                case CABECACIMA:
                    cabecaNova = '<';
                    jogo.jogada.qtdMovEsq++;
                    break;
                case CABECABAIXO:
                    cabecaNova = '>';
                    jogo.jogada.qtdMovDir++;
                    break;
            }
            break;
        case CONTINUE:
            cabecaNova = jogo.mapa.dirCabecaAnterior;
            switch (jogo.mapa.dirCabecaAnterior) {
                case CABECADIREITA:
                    jogo.jogada.qtdMovDir++;
                    break;
                case CABECAESQUERDA:
                    jogo.jogada.qtdMovEsq++;
                    break;
                case CABECACIMA:
                    jogo.jogada.qtdMovCima++;
                    break;
                case CABECABAIXO:
                    jogo.jogada.qtdMovBaixo++;
                    break;
            }
            break;
    }
    jogo.mapa = TransformaCorpoEmVazio(jogo.mapa, jogo.mapa.cabeca); // TIRA A CABECA DA POSICAO ANTERIOR
    jogo.mapa.cabecaAntiga = jogo.mapa.cabeca; // ESTABELECE A CABECA ATUAL COMO ANTIGA PARA A PROXIMA JOGADA
    jogo.mapa.cabeca = jogo.jogada.posProxCelula; // ATUALIZA POSICAO CABECA
    jogo.mapa.matriz[jogo.jogada.posProxCelula.y][jogo.jogada.posProxCelula.x] = cabecaNova; // COLOCA CARACTERE DA CABECA NOVA NA POSICAO NOVA

    return jogo;
}

int PosicoesSaoDiferentes(tPosicao posicao1, tPosicao posicao2) { // RETORNA SE POSICOES SAO DIFERENTES
    if ((posicao1.x != posicao2.x)) {
        return 1;
    }
    if ((posicao1.y != posicao2.y)) {
        return 1;
    }
    return 0;
}

tJogo RealizaJogada(tJogo jogo) { // FAZ TODA A JOGADA DESDE LER O MOVIMENTO ATE 

    //LE A DIRECAO DO MOVIMENTO
    jogo.jogada = LeMovimento(jogo.jogada);
    jogo.jogada.qtdMovimentos++;


    // QUAL A POSICAO NO MAPA DA CELULA QUE A COBRA IRA?
    jogo.jogada.posProxCelula = ChecaPosProxCelula(jogo.mapa, jogo.jogada);

    if (CoordenadaEstaForaDaMatriz(jogo.mapa, jogo.jogada.posProxCelula)) {
        jogo.jogada.posProxCelula = AtravessaLados(jogo.mapa, jogo.jogada);
    }


    //QUAL CARACTERE NA CELULA DA CABECA
    jogo.mapa.dirCabecaAnterior = QualCaractereEstaNaCelula(jogo.mapa, jogo.mapa.cabeca);


    // QUAL O CARACTERE NA CELULA QUE A CABECA IRA?
    jogo.jogada.caractereProxCelula = QualCaractereEstaNaCelula(jogo.mapa, jogo.jogada.posProxCelula);


    //ANDA SO A CABECA
    jogo = AndaCucaCobra(jogo);


    // ANDA CORPO
    jogo.mapa = AtualizaCorpoCobra(jogo.mapa);


    // FAZ AS ACOES DE ACORDO COM O TIPO DE CARACTERE DA PROX CELULA
    jogo = FazAcoesDeColisoes(jogo);

    if (PosicoesSaoDiferentes(jogo.mapa.rabo, jogo.mapa.raboAntigo)) { // REESTABELECE RABO
        jogo.mapa.raboAntigo = jogo.mapa.rabo;
    }
    jogo.mapa.cabeca = jogo.jogada.posProxCelula; // REESTABELECE CABECA


    //ATUALIZA HEATMAP
    jogo.heatMap.matriz[jogo.mapa.cabeca.y][jogo.mapa.cabeca.x] += 1;


    //VENCEU?
    if (!TemComidaAinda(jogo.mapa)) {
        jogo.flagEhFim = VENCEU;
    }


    // FAZ RESUMO DOS MOVIMENTOS QUE AUMENTARAM PONTUACAO OU FINALIZARAM O JOGO
    if ((jogo.jogada.caractereProxCelula != ' ')&&(!jogo.flagEhRabo)) {
        CriaResumo(jogo, jogo.arg1);
    }

    jogo.flagEhRabo = 0; // RESETA A FLAG EHRABO
    return jogo;
}

int TemComidaAinda(tMapa mapa) { // RETORNA SE TEM COMIDAS RESTANTES NO MAPA
    int i = 0, j = 0;
    for (i = 0; i < mapa.linhas; i++) {
        for (j = 0; j < mapa.colunas; j++) {
            if (mapa.matriz[i][j] == COMIDA) {
                return 1;
            }
        }
    }
    return 0;
}

void GeraRanking(tJogo jogo) { // FAZ UM RANKING DAS POSICOES MAIS FREQUENTADAS PELA COBRA EM ORDEM DECRESCENTE
    int contLinhas = 0, contColunas = 0, linhas = 0, colunas = 0, maiorNumero = 0, contRanking = 0, cont = 0, contMaior = 0;
    FILE * arquivo;
    char diretorio[DIRETORIO];
    tPosicao posMaiorNumero;

    linhas = RetornaQtdLinhasMapa(jogo.mapa);
    colunas = RetornaQtdColunasMapa(jogo.mapa);

    //ABRE ARQUIVO RANKING.TXT
    sprintf(diretorio, "%s/saida/ranking.txt", jogo.arg1); //ABRE ARQUIVO RANKING.TXT
    arquivo = fopen(diretorio, "w");

    //ACHA A QTD DE PASSADAS PELA CELULA MAIS FREQUENTADA
    for (contLinhas = 0; contLinhas < linhas; contLinhas++) {
        for (contColunas = 0; contColunas < colunas; contColunas++) {
            if (jogo.heatMap.matriz[contLinhas][contColunas] > maiorNumero) {
                maiorNumero = jogo.heatMap.matriz[contLinhas][contColunas];
            }
        }
    }

    //PROCURA TODAS AS OCORRENCIAS DA MAIOR QUANTIDADE ATE 1 NO HEATMAP
    for (contMaior = maiorNumero; contMaior > 0; contMaior--) {
        // VARRE A MATRIZ E PRINTA QTD E POSICAO REFERENTE NO ARQUIVO
        for (contLinhas = 0; contLinhas < linhas; contLinhas++) {
            for (contColunas = 0; contColunas < colunas; contColunas++) {
                if (jogo.heatMap.matriz[contLinhas][contColunas] == contMaior) {
                    fprintf(arquivo, "(%d, %d) - ", contLinhas, contColunas);
                    fprintf(arquivo, "%d\n", contMaior);
                }
            }
        }
    }

    fclose(arquivo); // FECHA ARQUIVO RANKING.TXT
}

void GeraEstatisticas(tJogo jogo) { // PRINTA AS ESTATISTICAS SOBRE A MOVIMENTACAO DA COBRINHA
    FILE * arquivo;
    char diretorio[DIRETORIO];

    // ABRE ARQUIVO ESTATISTICAS.TXT
    sprintf(diretorio, "%s/saida/estatisticas.txt", jogo.arg1); //ABRE ARQUIVO ESTATISTICAS.TXT
    arquivo = fopen(diretorio, "w");

    // PRINTA ESTATISTICAS COLETADAS ANTERIORMENTE NO ARQUIVO
    fprintf(arquivo, "Numero de movimentos: %d\n", jogo.jogada.qtdMovimentos);
    fprintf(arquivo, "Numero de movimentos sem pontuar: %d\n", jogo.jogada.qtdMovSemPontuar);
    fprintf(arquivo, "Numero de movimentos para baixo: %d\n", jogo.jogada.qtdMovBaixo);
    fprintf(arquivo, "Numero de movimentos para cima: %d\n", jogo.jogada.qtdMovCima);
    fprintf(arquivo, "Numero de movimentos para esquerda: %d\n", jogo.jogada.qtdMovEsq);
    fprintf(arquivo, "Numero de movimentos para direita: %d\n", jogo.jogada.qtdMovDir);

    fclose(arquivo); // FECHA ARQUIVO ESTATISTICAS.TXT
}

tMapa CobraViraX(tMapa mapa) { // TRANSFORMA CABECA E CORPO DA COBRA EM X 
    int i = 0, j = 0;
    for (i = 0; i < mapa.linhas; i++) {
        for (j = 0; j < mapa.colunas; j++) {
            if ((mapa.matriz[i][j] == CORPO) || EhCabeca((mapa.matriz[i][j]))) {

                mapa.matriz[i][j] = CARCACA;
            }
        }
    }
    return mapa;
}

int EhFimDoJogo(tJogo jogo) { // CHECA SE O USUARIO PERDEU OU GANHOU O JOGO NO ESTADO ATUAL
    if (jogo.flagEhFim == VENCEU || jogo.flagEhFim == PERDEU) {
        return 1;
    }
    return 0;
}

void FinalizaJogo(tJogo jogo) { // GERA ARQUIVOS FINAIS E PRINTA O ULTIMO ESTADO DO JOGO, MOSTRA SE GANHOU OU PERDEU E PONTUACAO FINAL

    //ARQUIVOS FINAIS
    PrintaMatrizHeat(jogo);
    GeraRanking(jogo);
    GeraEstatisticas(jogo);

    // VITORIA
    if (jogo.flagEhFim == VENCEU) {
        PrintaEstadoDoJogo(jogo);
        printf("Voce venceu!\n");
    }// DERROTA
    else if (jogo.flagEhFim == PERDEU) {
        jogo.mapa = CobraViraX(jogo.mapa);
        PrintaEstadoDoJogo(jogo);
        printf("Game over!\n");
    }
    printf("Pontuacao final: %d\n", jogo.pontuacao);
}

int main(int argc, char * argv[]) {
    tJogo jogo;
    char movimento = 's', enter = 'd';
    int i = 0;

    // AVISA SE O USUARIO NAO OFERECEU MAIS DE 1 ARGUMENTO NO TERMINAL
    if (argc <= 1) {
        printf("ERRO: O diretorio de arquivos de configuracao nao foi informado!\n");
        return 1;
    }

    //COLOCA DIRETORIO DADO NUMA VARIAVEL DE TIPO JOGO, PARA SER UTILIZADA POR TODOS ARQUIVOS
    sprintf(jogo.arg1, "%s", argv[1]);

    // CRIA ARQUIVO INICIALIZACAO.TXT COM MATRIZ E POSICAO INICIAL DA CABECA DA COBRA
    jogo = InicializaJogo(jogo);

    // FAZ AS JOGADAS ATE QUE SEJA O FIM DO JOGO
    while (!EhFimDoJogo(jogo)) {
        jogo = RealizaJogada(jogo);
        if (!EhFimDoJogo(jogo)) { // DEIXA PRA PRINTAR O ESTADO DO JOGO FINAL NA FUNCAO FINALIZAJOGO
            PrintaEstadoDoJogo(jogo);
        }
    }

    // GERA ARQUIVOS FINAIS E PRINTA O ULTIMO ESTADO DO JOGO, MOSTRA SE GANHOU OU PERDEU E PONTUACAO FINAL
    FinalizaJogo(jogo);

    // FECHA ARQUIVOS
    jogo = FechaArquivos(jogo);
    return 0;
}
