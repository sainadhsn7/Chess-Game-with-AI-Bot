#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "move.h"
#include <QList>
#include "board.h"
#include "precomputedmovedata.h"
#include "bitboard.h"

typedef BoardRepresentation BR;

class MoveGenerator
{
public:
    enum PromotionMode{ All, QueenOnly, QueenAndKnight };

    PromotionMode promotionsToGenerate = All;

    uint64_t opponentAttackMap;
    uint64_t opponentPawnAttackMap;

    MoveGenerator();
    // Copy constructor
    MoveGenerator(const MoveGenerator &other);

    // Move constructor
    MoveGenerator(MoveGenerator &&other) noexcept;

    // Copy assignment operator
    MoveGenerator& operator=(const MoveGenerator &other);

    // Move assignment operator
    MoveGenerator& operator=(MoveGenerator &&other) noexcept;

    QList<Move> GenerateMoves(Board *board, bool includeQuietMoves=true);
    bool InCheck () {
        return inCheck;
    }
private:
    // ---- Instance variables ----
    void printdebug();
    PrecomputedMoveData PMD;
    QList<Move> moves;
    bool isWhiteToMove;
    int friendlyColour;
    int opponentColour;
    int friendlyKingSquare;
    int friendlyColourIndex;
    int opponentColourIndex;

    bool inCheck;
    bool inDoubleCheck;
    bool pinsExistInPosition;

    uint64_t checkRayBitmask;
    uint64_t pinRayBitmask;
    uint64_t opponentKnightAttacks;
    uint64_t opponentAttackMapNoPawns;
    uint64_t opponentSlidingAttackMap;

    bool genQuiets;
    Board* board;

    void Init();
    void GenerateKingMoves();
    void GenerateKnightMoves();
    void GenerateSlidingMoves();
    void GenerateSlidingPieceMoves(int startSquare, int startDirIndex, int endDirIndex);
    void GeneratePawnMoves();
    void MakePromotionMoves (int fromSquare, int toSquare);

    bool IsMovingAlongRay (int rayDir, int startSquare, int targetSquare) {
        int moveDir = PMD.directionLookup[targetSquare - startSquare + 63];
        return (rayDir == moveDir || -rayDir == moveDir);
    }

    //bool IsMovingAlongRay (int directionOffset, int absRayOffset) {
    //return !((directionOffset == 1 || directionOffset == -1) && absRayOffset >= 7) && absRayOffset % directionOffset == 0;
    //}

    bool IsPinned (int square) {
        return (pinsExistInPosition && ((pinRayBitmask >> square) & 1)) != 0;
    }

    bool SquareIsInCheckRay (int square) {
        return (inCheck && ((checkRayBitmask >> square) & 1)) != 0;
    }

    bool HasKingsideCastleRight() {
        int mask = (board->WhiteToMove) ? 1 : 4;
        return (board->currentGameState & mask) != 0;
    }

    bool HasQueensideCastleRight() {
        int mask = (board->WhiteToMove) ? 2 : 8;
        return (board->currentGameState & mask) != 0;
    }
    bool SquareIsAttacked (int square) {
        return BitBoard::ContainsSquare (opponentAttackMap, square);
    }
    void GenSlidingAttackMap ();
    void UpdateSlidingAttackPiece (int startSquare, int startDirIndex, int endDirIndex);
    void CalculateAttackData ();
    bool InCheckAfterEnPassant (int startSquare, int targetSquare, int epCapturedPawnSquare);
    bool SquareAttackedAfterEPCapture (int epCaptureSquare, int capturingPawnStartSquare);
};
#endif // MOVEGENERATOR_H
