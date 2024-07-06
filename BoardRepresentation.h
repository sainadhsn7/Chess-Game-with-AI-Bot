#ifndef BOARDREPRESENTATION_H
#define BOARDREPRESENTATION_H

#include <QString>
#include "Coord.h"
class BoardRepresentation{
public:
    static const QString fileNames;
    static const QString rankNames;

    static const int a1 = 0;
    static const int b1 = 1;
    static const int c1 = 2;
    static const int d1 = 3;
    static const int e1 = 4;
    static const int f1 = 5;
    static const int g1 = 6;
    static const int h1 = 7;

    static const int a8 = 56;
    static const int b8 = 57;
    static const int c8 = 58;
    static const int d8 = 59;
    static const int e8 = 60;
    static const int f8 = 61;
    static const int g8 = 62;
    static const int h8 = 63;

    static int RankIndex (int squareIndex) {
        return squareIndex >> 3;
    }

    static int FileIndex (int squareIndex) {
        return squareIndex & 0b000111;
    }

    static int IndexFromCoord (int file, int rank) {
        return (rank * 8) + file;
    }

    static int IndexFromCoord (Coord coord) {
        return IndexFromCoord (coord.fileIndex, coord.rankIndex);
    }

    static Coord CoordFromIndex (int squareIndex) {
        return Coord(RankIndex(squareIndex), FileIndex(squareIndex));
    }

    static bool LightSquare (int col, int row) {
        return (col + row) % 2 != 0;
    }

    static QString SquareNameFromCoordinate (int col, int row) {
        return QString(fileNames[col]) + QString::number(row + 1);
    }

    static QString SquareNameFromIndex (int squareIndex) {
        return SquareNameFromCoordinate (CoordFromIndex(squareIndex));
    }

    static QString SquareNameFromCoordinate (Coord coord) {
        return SquareNameFromCoordinate (coord.fileIndex, coord.rankIndex);
    }
};

#endif // BOARDREPRESENTATION_H
