#ifndef PIECELIST_H
#define PIECELIST_H

#include <QGraphicsSvgItem>
#include <QMap>
class PieceList
{
public:
    // Indices of squares occupied by given piece type (only elements up to Count are valid, the rest are unused/garbage)
    QList<int> occupiedSquares;
    // Map to go from index of a square, to the index in the occupiedSquares array where that square is stored
    int map[64];
    int numPieces;

    PieceList (int maxPieceCount = 16);
    int Count();

    void AddPieceAtSquare (int square);

    void RemovePieceAtSquare (int square);

    void MovePiece (int startSquare, int targetSquare);

    int operator[](int index) const {
        if (index < 0 || index >= 15) {
            throw std::out_of_range("Index out of range");
        }
        return occupiedSquares[index];
    }
};

#endif // PIECELIST_H
