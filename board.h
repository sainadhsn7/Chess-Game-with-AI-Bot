#ifndef BOARD_H
#define BOARD_H

#include "piece.h"
#include "piecelist.h"
#include <QStack>
#include "move.h"

class FenNotation;
class Zobrist;
class Board
{
public:
    static const int squareLen = 80;
    static const int boardLen = 8*squareLen;
    int Square[64];
    static const int WhiteIndex = 0;
    static const int BlackIndex = 1;

    // Stores piece code for each square on the board.
    // Piece code is defined as piecetype | colour code
    bool WhiteToMove;
    int ColourToMove;
    int OpponentColour;
    int ColourToMoveIndex;
    // Bits 0-3 store white and black kingside/queenside castling legality
    // Bits 4-7 store file of ep square (starting at 1, so 0 = no ep square)
    // Bits 8-13 captured piece
    // Bits 14-... fifty mover counter
    uint32_t currentGameState;

    int plyCount; // Total plies played in game
    int fiftyMoveCounter; // Num ply since last pawn move or capture

    uint64_t ZobristKey;
    /// List of zobrist keys
    QStack<unsigned long long> RepetitionPositionHistory;

    int KingSquare[2]; // index of square of white and black king

    PieceList rooks[2];
    PieceList bishops[2];
    PieceList queens[2];
    PieceList knights[2];
    PieceList pawns[2];

    Board();
    ~Board();
    void loadStartPosition();
    void loadPosition(QString fen);
    void makeMove(Move &move, bool inSearch = false);
    void unmakeMove(Move &move, bool inSearch = false);
private:

    QStack<uint32_t> gameStateHistory;
    QList<PieceList *>allPieceLists;

    static const uint32_t whiteCastleKingsideMask = 0b1111111111111110;
    static const uint32_t whiteCastleQueensideMask = 0b1111111111111101;
    static const uint32_t blackCastleKingsideMask = 0b1111111111111011;
    static const uint32_t blackCastleQueensideMask = 0b1111111111110111;

    static const uint32_t whiteCastleMask = whiteCastleKingsideMask & whiteCastleQueensideMask;
    static const uint32_t blackCastleMask = blackCastleKingsideMask & blackCastleQueensideMask;

    Piece Piece;
    Zobrist *Zobrist;

    void Initialize();

    PieceList* GetPieceList (int pieceType, int colourIndex) {
        return allPieceLists[colourIndex * 8 + pieceType];
    }
};

#endif // BOARD_H
