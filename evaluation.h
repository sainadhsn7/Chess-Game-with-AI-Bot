#ifndef EVALUATION_H
#define EVALUATION_H

#include "board.h"
#include "piecesquaretable.h"
#include "precomputedmovedata.h"

class Evaluation
{
public:
    static const int pawnValue = 100;
    static const int knightValue = 300;
    static const int bishopValue = 320;
    static const int rookValue = 500;
    static const int queenValue = 900;

    // Performs static evaluation of the current position.
    // The position is assumed to be 'quiet', i.e no captures are available that could drastically affect the evaluation.
    // The score that's returned is given from the perspective of whoever's turn it is to move.
    // So a positive score means the player who's turn it is to move has an advantage, while a negative score indicates a disadvantage.
    int Evaluate (Board booard);

    static int EvaluatePieceSquareTable (const int table[64], PieceList pieceList, bool isWhite) {
        int value = 0;
        for (int i = 0; i < pieceList.Count(); i++) {
            value += PieceSquareTable::Read (table, pieceList[i], isWhite);
        }
        return value;
    }
    Evaluation();
private:
    PrecomputedMoveData PMD;
    const float endgameMaterialStart = rookValue * 2 + bishopValue + knightValue;
    Board board;
    float EndgamePhaseWeight (int materialCountWithoutPawns) {
        const float multiplier = 1 / endgameMaterialStart;
        return 1 - fmin (1, materialCountWithoutPawns * multiplier);
    }

    int MopUpEval (int friendlyIndex, int opponentIndex, int myMaterial, int opponentMaterial, float endgameWeight);
    int CountMaterial (int colourIndex);
    int EvaluatePieceSquareTables (int colourIndex, float endgamePhaseWeight);
};

#endif // EVALUATION_H
