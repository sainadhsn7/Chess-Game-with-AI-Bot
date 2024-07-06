#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H

#include "Player.h"
#include "Coord.h"
#include "board.h"
#include "qapplication.h"
#include <boardui.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>

class HumanPlayer :public Player
{
public:
    enum InputState {
        None,
        PieceSelected,
        DraggingPiece
    };

    InputState currentState;
    Coord *selectedPieceSquare;
    Board *board;
    BoardUI *boardUI;

    HumanPlayer (Board *board, BoardUI* boardUI, QObject *par=nullptr) :Player(par){
        this->board = board;
        this->boardUI = boardUI;
        currentState = None;
        selectedPieceSquare = new Coord(0, 0);
    }
    ~HumanPlayer()override{
        delete selectedPieceSquare;
    }

    void NotifyTurnToMove () override{

    }
    void Update () override{
        HandleInput ();
    }
private:
    void HandleInput ();
    void HandlePointAndClickMovement (QPointF mousePos);
    void HandleDragMovement (QPointF mousePos);
    void HandlePiecePlacement (QPointF mousePos);
    void CancelPieceSelection ();
    void TryMakeMove (Coord startSquare, Coord targetSquare);
    void HandlePieceSelection (QPointF mousePos);
};

#endif // HUMANPLAYER_H
