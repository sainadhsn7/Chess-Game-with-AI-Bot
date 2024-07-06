#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <random>
#include <QQueue>
#include "board.h"
class Board;
class Zobrist
{
public:
     const int seed = 2361912;
    QString randomNumbersFileName;
    //  QVector<QVector<QVector<uint64_t, 64>, 2>, 8> piecesArray;
    QVector<QVector<QVector<uint64_t>>> piecesArray;
    QVector<uint64_t> castlingRights;
    QVector<uint64_t> enPassantFile;
    uint64_t sideToMove;
    
    uint64_t calculateZobristKey(Board *board);
    Zobrist();
    
private:
     std::mt19937_64 prng;
     QString randomNumbersPath;
    void writeRandomNumbers();
    QQueue<uint64_t> readRandomNumbers();
    uint64_t randomUnsigned64BitNumber();
};

#endif // ZOBRIST_H
