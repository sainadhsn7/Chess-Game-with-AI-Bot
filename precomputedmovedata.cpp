#include "precomputedmovedata.h"
#include "board.h"

//all static variables


PrecomputedMoveData::PrecomputedMoveData() {
    qDebug() << "PMD init called";
    // Calculate knight jumps and available squares for each square on the board.
    // See comments by variable definitions for more info.
    int allKnightJumps[] = { 15, 17, -17, -15, 10, -6, 6, -10 };

    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {

        int y = squareIndex / 8;
        int x = squareIndex - (y * 8);

        int north = 7 - y;
        int south = y;
        int west = x;
        int east = 7 - x;
        numSquaresToEdge[squareIndex][0] = north;
        numSquaresToEdge[squareIndex][1] = south;
        numSquaresToEdge[squareIndex][2] = west;
        numSquaresToEdge[squareIndex][3] = east;
        numSquaresToEdge[squareIndex][4] = qMin (north, west);
        numSquaresToEdge[squareIndex][5] = qMin (south, east);
        numSquaresToEdge[squareIndex][6] = qMin (north, east);
        numSquaresToEdge[squareIndex][7] = qMin (south, west);

        // Calculate all squares knight can jump to from current square
        QList<byte> legalKnightJumps;
        uint64_t knightBitboard = 0;
        for(int knightJumpDelta: allKnightJumps) {
            int knightJumpSquare = squareIndex + knightJumpDelta;
            if (knightJumpSquare >= 0 && knightJumpSquare < 64) {
                int knightSquareY = knightJumpSquare / 8;
                int knightSquareX = knightJumpSquare - (knightSquareY * 8);
                // Ensure knight has moved max of 2 squares on x/y axis (to reject indices that have wrapped around side of board)
                int maxCoordMoveDst = qMax (abs(x - knightSquareX), abs(y - knightSquareY));
                if (maxCoordMoveDst == 2) {
                    legalKnightJumps.push_back((byte) knightJumpSquare);
                    knightBitboard |= 1ull << knightJumpSquare;
                }
            }
        }
        knightMoves[squareIndex] = legalKnightJumps;
        knightAttackBitboards[squareIndex] = knightBitboard;

        // Calculate all squares king can move to from current square (not including castling)
        QList<byte> legalKingMoves;
        for (int kingMoveDelta : directionOffsets) {
            int kingMoveSquare = squareIndex + kingMoveDelta;
            if (kingMoveSquare >= 0 && kingMoveSquare < 64) {
                int kingSquareY = kingMoveSquare / 8;
                int kingSquareX = kingMoveSquare - kingSquareY * 8;
                // Ensure king has moved max of 1 square on x/y axis (to reject indices that have wrapped around side of board)
                int maxCoordMoveDst = qMax (abs(x - kingSquareX), abs(y - kingSquareY));
                if (maxCoordMoveDst == 1) {
                    legalKingMoves.push_back((byte) kingMoveSquare);
                    kingAttackBitboards[squareIndex] |= 1ull << kingMoveSquare;
                }
            }
        }
        kingMoves[squareIndex] = legalKingMoves;

        // Calculate legal pawn captures for white and black
        QList<int> pawnCapturesWhite;
        QList<int> pawnCapturesBlack;
        if (x > 0) {
            if (y < 7) {
                pawnCapturesWhite.push_back(squareIndex + 7);
                pawnAttackBitboards[squareIndex][Board::WhiteIndex] |= 1ull << (squareIndex + 7);
            }
            if (y > 0) {
                pawnCapturesBlack.push_back(squareIndex - 9);
                pawnAttackBitboards[squareIndex][Board::BlackIndex] |= 1ull << (squareIndex - 9);
            }
        }
        if (x < 7) {
            if (y < 7) {
                pawnCapturesWhite.push_back(squareIndex + 9);
                pawnAttackBitboards[squareIndex][Board::WhiteIndex] |= 1ull << (squareIndex + 9);
            }
            if (y > 0) {
                pawnCapturesBlack.push_back(squareIndex - 7);
                pawnAttackBitboards[squareIndex][Board::BlackIndex] |= 1ull << (squareIndex - 7);
            }
        }
        // pawnAttacksWhite[squareIndex] = pawnCapturesWhite;
        pawnAttacksWhite[squareIndex] = pawnCapturesWhite;
        pawnAttacksBlack[squareIndex] = pawnCapturesBlack;

        // Rook moves
        for (int directionIndex = 0; directionIndex < 4; directionIndex++) {
            int currentDirOffset = directionOffsets[directionIndex];
            for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++) {
                int targetSquare = squareIndex + currentDirOffset * (n + 1);
                rookMoves[squareIndex] |= 1ull << targetSquare;
            }
        }
        // Bishop moves
        for (int directionIndex = 4; directionIndex < 8; directionIndex++) {
            int currentDirOffset = directionOffsets[directionIndex];
            for (int n = 0; n < numSquaresToEdge[squareIndex][directionIndex]; n++) {
                int targetSquare = squareIndex + currentDirOffset * (n + 1);
                bishopMoves[squareIndex] |= 1ull << targetSquare;
            }
        }
        queenMoves[squareIndex] = rookMoves[squareIndex] | bishopMoves[squareIndex];
    }

    for (int i = 0; i < 127; i++) {
        int offset = i - 63;
        int absOffset = abs(offset);
        int absDir = 1;
        if (absOffset % 9 == 0) {
            absDir = 9;
        } else if (absOffset % 8 == 0) {
            absDir = 8;
        } else if (absOffset % 7 == 0) {
            absDir = 7;
        }

        directionLookup[i] = absDir * ((offset>0)-(offset<0));
    }

    // Distance lookup
    for (int squareA = 0; squareA < 64; squareA++) {
        Coord coordA = BoardRepresentation::CoordFromIndex (squareA);
        int fileDstFromCentre = qMax (3 - coordA.fileIndex, coordA.fileIndex - 4);
        int rankDstFromCentre = qMax (3 - coordA.rankIndex, coordA.rankIndex - 4);
        centreManhattanDistance[squareA] = fileDstFromCentre + rankDstFromCentre;

        for (int squareB = 0; squareB < 64; squareB++) {

            Coord coordB = BoardRepresentation::CoordFromIndex (squareB);
            int rankDistance = abs (coordA.rankIndex - coordB.rankIndex);
            int fileDistance = abs (coordA.fileIndex - coordB.fileIndex);
            orthogonalDistance[squareA][squareB] = fileDistance + rankDistance;
            kingDistance[squareA][squareB] = qMax (fileDistance, rankDistance);
        }
    }
    qDebug() << "PMD init complete";
}
