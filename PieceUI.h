#ifndef PIECEUI_H
#define PIECEUI_H

#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include <piece.h>
#include <BoardRepresentation.h>
#include <QDebug>
#include <QPointF>

typedef BoardRepresentation BR;
class PieceUI{
public:
    int PieceVal = 0;
    QGraphicsSvgItem *UIPiece = nullptr;
    PieceUI(int PieceVal, const int squareLen = 80){

        this->PieceVal = PieceVal;
        this->squareLen = squareLen;
        UIPiece = getPiece();
    }
    ~PieceUI(){
        delete UIPiece;
        delete renderer;
        UIPiece = nullptr;
        renderer = nullptr;
    }
    void SetPosition(QPointF point){
        UIPiece->setPos(point);
        UIPiece->setZValue(pieceDepth);
    }

private:
    int squareLen;

    const float pieceDepth = 0.1f;

    QSvgRenderer *renderer = nullptr;

    QGraphicsSvgItem* createSvgItem(const QString& filePath, int squareSize) {
        renderer = new QSvgRenderer(filePath);
        QGraphicsSvgItem *svgItem = new QGraphicsSvgItem();
        svgItem->setSharedRenderer(renderer);
        svgItem->setScale(squareSize / 2000.0); // assuming the SVG's viewport is 2000x2000
        return svgItem;
    }
    QGraphicsSvgItem* getPiece(){
        bool isWhite = Piece::Colour(PieceVal) == Piece::White;
        QString pieceFilePath;
        int type = Piece::PieceType(PieceVal);

        if(type == Piece::Pawn)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wP.svg" : ":/piece/chesspieces/bP.svg";
        if(type == Piece::Knight)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wN.svg" : ":/piece/chesspieces/bN.svg";
        if(type == Piece::Bishop)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wB.svg" : ":/piece/chesspieces/bB.svg";
        if(type == Piece::Rook)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wR.svg" : ":/piece/chesspieces/bR.svg";
        if(type == Piece::Queen)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wQ.svg" : ":/piece/chesspieces/bQ.svg";
        if(type == Piece::King)
            pieceFilePath = isWhite ? ":/piece/chesspieces/wK.svg" : ":/piece/chesspieces/bK.svg";

        QGraphicsSvgItem *newPiece = createSvgItem(pieceFilePath, squareLen);

        return newPiece;
    }
};

#endif // PIECEUI_H
