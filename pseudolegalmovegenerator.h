#ifndef PSEUDOLEGALMOVEGENERATOR_H
#define PSEUDOLEGALMOVEGENERATOR_H

#include "move.h"
#include "board.h"
#include "precomputedmovedata.h"
#include "BoardRepresentation.h"

typedef BoardRepresentation BR;

class PseudoLegalMoveGenerator
{
private:
    PrecomputedMoveData PMD;
    QList<Move> moves;
    bool isWhiteToMove;
    int friendlyColour;
    int opponentColour;
    int friendlyKingSquare;
    int friendlyColourIndex;
    int opponentColourIndex;

    bool genQuiets;
    bool genUnderpromotions;
    Board *board;

    void Init();
    void GenerateKingMoves ();
    void GenerateSlidingMoves ();
    void GenerateSlidingPieceMoves (int startSquare, int startDirIndex, int endDirIndex);
    void GenerateKnightMoves ();
    void GeneratePawnMoves ();
    void MakePromotionMoves (int fromSquare, int toSquare);
    bool HasKingsideCastleRight() {
        int mask = (board->WhiteToMove) ? 1 : 4;
        return (board->currentGameState & mask) != 0;
    }

    bool HasQueensideCastleRight() {
        int mask = (board->WhiteToMove) ? 2 : 8;
        return (board->currentGameState & mask) != 0;
    }
public:
    // Generates list of legal moves in current position.
    // Quiet moves (non captures) can optionally be excluded. This is used in quiescence search.
    QList<Move> GenerateMoves (Board *board, bool includeQuietMoves = true, bool includeUnderPromotions = true);
    bool Illegal () {
        return SquareAttacked (board->KingSquare[1 - board->ColourToMoveIndex], board->ColourToMove);
    }
    bool SquareAttacked (int attackSquare, int attackerColour);

    // Note, this will only return correct value after GenerateMoves() has been called in the current position
    bool InCheck () {
        return false;
        //return SquareAttacked (friendlyKingSquare, board.ColourToMoveIndex);
    }
    PseudoLegalMoveGenerator();
};

#endif // PSEUDOLEGALMOVEGENERATOR_H
