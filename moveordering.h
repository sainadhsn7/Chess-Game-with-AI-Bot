#ifndef MOVEORDERING_H
#define MOVEORDERING_H

#include "transpositiontable.h"
#include "movegenerator.h"
#include "evaluation.h"
#include <QtGlobal>

class MoveOrdering
{
public:
    MoveOrdering (MoveGenerator moveGenerator, TranspositionTable *tt);
    ~MoveOrdering();

    void OrderMoves (Board board, QList<Move> moves, bool useTT);

    static int GetPieceValue (int pieceType) {
        switch (pieceType) {
        case Piece::Queen:
            return Evaluation::queenValue;
        case Piece::Rook:
            return Evaluation::rookValue;
        case Piece::Knight:
            return Evaluation::knightValue;
        case Piece::Bishop:
            return Evaluation::bishopValue;
        case Piece::Pawn:
            return Evaluation::pawnValue;
        default:
            return 0;
        }
    }
private:
    QList<int> moveScores;
    const int maxMoveCount = 218;

    const int squareControlledByOpponentPawnPenalty = 350;
    const int capturedPieceValueMultiplier = 10;

    MoveGenerator moveGenerator;
    TranspositionTable *transpositionTable;

    Move invalidMove = Move::InvalidMove();
    void Sort (QList<Move> moves);
};

#endif // MOVEORDERING_H
