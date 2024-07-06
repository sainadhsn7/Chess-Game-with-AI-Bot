#ifndef BOARDUI_H
#define BOARDUI_H

#include <QGraphicsView>
#include "movegenerator.h"
#include "board.h"
#include "PieceUI.h"
#include "Coord.h"
#include <BoardTheme.h>

class MainWindow;
class BoardUI
{
public:
    bool whiteIsBottom = true;
    bool Highlight = true;
    bool showLegalMoves = true;

    Theme boardTheme;

    QGraphicsScene * scene;
    QGraphicsView *view;

    PieceUI *SquarePiece[64];
    void ResetSquareColor ();
    void setPerspective(bool whitePov);
    void updatePosition(Board* board);

    void selectSquare(Coord coord);
    void deselectSquare(Coord coord);
    void HighlightLegalMoves (Board *board, Coord fromSquare);

    void DragPiece(Coord pieceCoord, QPointF mousePos);
    void ResetPiecePosition(Coord pieceCoord);
    bool TryGetSquareUnderMouse(QPointF mouseWorld, Coord &selectedCoord);
    void OnMoveMade(Board *board, Move &move, bool animate = false);
    void AnimateMove(Move &move, Board *board);

    QPointF PositionFromCoord(int file, int rank);
    QPointF PositionFromCoord(Coord coord);
    BoardUI(MainWindow *Parent):parWindow(Parent){
        for(int i=0; i<64; i++){
            SquarePiece[i]=nullptr;
            Square[i] = nullptr;
        }
        CreateBoardUI();
    }
    ~BoardUI();
private:
    MainWindow *parWindow;

    QGraphicsRectItem *Square[64];

    Move lastMadeMove = Move::InvalidMove();
    MoveGenerator moveGenerator;

    QColor lightSquareCol = QColor("#F0D9B5"), darkSquareCol = QColor("#B58863"), lastFromCol = QColor("#1E90FF"), LastToCol = QColor("#FFA500");
    QColor SelectedSquareCol = QColor("#FFA500");

    const float pieceDepth = 0.1f;
    const float pieceDragDepth = 0.2f;

    void CreateBoardUI();
    void setSquareColor(Coord square, QColor light, QColor dark);
    int boardLen, squareLen = 80;
    void ResetSquarePosition();
    void HighlightMove(Move &move);

};

#endif // BOARDUI_H
