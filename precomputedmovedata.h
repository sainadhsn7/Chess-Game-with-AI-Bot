#ifndef PRECOMPUTEDMOVEDATA_H
#define PRECOMPUTEDMOVEDATA_H

#include <cstdint>
#include <QList>

typedef uint8_t byte;

class PrecomputedMoveData
{
public:
    const int directionOffsets[8] = { 8, -8, -1, 1, 7, -7, 9, -9 };

    // Stores number of moves available in each of the 8 directions for every square on the board
    // Order of directions is: N, S, W, E, NW, SE, NE, SW
    // So for example, if availableSquares[0][1] == 7...
    // that means that there are 7 squares to the north of b1 (the square with index 1 in board array)
    int numSquaresToEdge[64][8];

    // Stores array of indices for each square a knight can land on from any square on the board
    // So for example, knightMoves[0] is equal to {10, 17}, meaning a knight on a1 can jump to c2 and b3
    QList<byte> knightMoves[64];
    QList<byte> kingMoves[64];

    // Pawn attack directions for white and black (NW, NE; SW SE)
    const byte pawnAttackDirections[2][2] = {
        { 4, 6 },
        { 7, 5 }
    };

    QList<int> pawnAttacksWhite[64];
    QList<int> pawnAttacksBlack[64];
    int directionLookup[127];

    uint64_t kingAttackBitboards[64];
    uint64_t knightAttackBitboards[64];
    uint64_t pawnAttackBitboards[64][2];

    uint64_t rookMoves[64];
    uint64_t bishopMoves[64];
    uint64_t queenMoves[64];

    // Aka manhattan distance (answers how many moves for a rook to get from square a to square b)
    int orthogonalDistance[64][64];
    // Aka chebyshev distance (answers how many moves for a king to get from square a to square b)
    int kingDistance[64][64];
    int centreManhattanDistance[64];

    int NumRookMovesToReachSquare (int startSquare, int targetSquare) {
        return orthogonalDistance[startSquare][targetSquare];
    }

    int NumKingMovesToReachSquare (int startSquare, int targetSquare) {
        return kingDistance[startSquare][targetSquare];
    }
    PrecomputedMoveData();
};

#endif // PRECOMPUTEDMOVEDATA_H
