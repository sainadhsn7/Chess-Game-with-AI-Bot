#include "piecelist.h"
#include <QGraphicsSvgItem>

PieceList::PieceList (int maxPieceCount) {
    occupiedSquares.resize(maxPieceCount);
    numPieces = 0;
}

int PieceList::Count() {
    return numPieces;
}

void PieceList::AddPieceAtSquare (int square) {
    occupiedSquares[numPieces] = square;
    map[square] = numPieces;
    numPieces++;
}

void PieceList::RemovePieceAtSquare (int square) {
    int pieceIndex = map[square]; // get the index of this element in the occupiedSquares array
    occupiedSquares[pieceIndex] = occupiedSquares[numPieces - 1]; // move last element in array to the place of the removed element
    map[occupiedSquares[pieceIndex]] = pieceIndex; // update map to point to the moved element's new location in the array
    numPieces--;
}

void PieceList::MovePiece (int startSquare, int targetSquare) {
    int pieceIndex = map[startSquare]; // get the index of this element in the occupiedSquares array
    occupiedSquares[pieceIndex] = targetSquare;
    map[targetSquare] = pieceIndex;
}
