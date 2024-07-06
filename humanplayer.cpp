#include "humanplayer.h"
#include <QDebug>
// HumanPlayer::HumanPlayer() {}
void HumanPlayer::HandleInput () {
    QPointF mousePos = boardUI->view->mapToScene(boardUI->view->mapFromGlobal(QCursor::pos()));
    // qDebug() << mousePos << " " <<  currentState;
    if (currentState == None) {
        HandlePieceSelection(mousePos);
    } else if (currentState == DraggingPiece) {
        HandleDragMovement(mousePos);
    } else if (currentState == PieceSelected) {
        HandlePointAndClickMovement(mousePos);
    }

    if (QApplication::mouseButtons() & Qt::RightButton) {
        CancelPieceSelection();
    }
}

void HumanPlayer::HandlePieceSelection (QPointF mousePos) {
    if (QApplication::mouseButtons() & Qt::LeftButton) {
        if (boardUI->TryGetSquareUnderMouse(mousePos, *selectedPieceSquare)) {
            int index = BoardRepresentation::IndexFromCoord(*selectedPieceSquare);
            if (Piece::IsColour(board->Square[index], board->ColourToMove)) {
                boardUI->HighlightLegalMoves(board, *selectedPieceSquare);
                boardUI->selectSquare(*selectedPieceSquare);
                currentState = DraggingPiece;
            }
        }
    }
}

void HumanPlayer::HandlePointAndClickMovement (QPointF mousePos) {

    if (QApplication::mouseButtons() & Qt::LeftButton) {
        HandlePiecePlacement(mousePos);
    }
}

void HumanPlayer::HandleDragMovement (QPointF mousePos) {
    boardUI->DragPiece(*selectedPieceSquare, mousePos);
    // // If mouse is released, then try place the piece
    if (!(QApplication::mouseButtons() & Qt::LeftButton)) {
        HandlePiecePlacement(mousePos);
    }
}

void HumanPlayer::HandlePiecePlacement (QPointF mousePos) {
    Coord *targetSquare = new Coord(0, 0);
    if (boardUI->TryGetSquareUnderMouse(mousePos, *targetSquare)) {
        // qDebug() << "(" << targetSquare->rankIndex << ", " << targetSquare->fileIndex << ")   " << "(" << selectedPieceSquare->rankIndex << ", " << selectedPieceSquare->fileIndex << ")";
        if (targetSquare->CompareTo( *selectedPieceSquare)) {
            boardUI->ResetPiecePosition(*selectedPieceSquare);
            if (currentState == DraggingPiece) {
                currentState = PieceSelected;
            } else {
                currentState = None;
                boardUI->deselectSquare(*selectedPieceSquare);
            }
        } else {
            int targetIndex = BoardRepresentation::IndexFromCoord(*targetSquare);
            if (Piece::IsColour(board->Square[targetIndex], board->ColourToMove) && board->Square[targetIndex] != 0) {
                CancelPieceSelection();
                HandlePieceSelection(mousePos);
            } else {
                TryMakeMove(*selectedPieceSquare, *targetSquare);
            }
        }
    } else {
        CancelPieceSelection();
    }

}

void HumanPlayer::CancelPieceSelection () {
    if (currentState != None) {
        currentState = None;
        boardUI->deselectSquare(*selectedPieceSquare);
        boardUI->ResetPiecePosition(*selectedPieceSquare);
    }
}

void HumanPlayer::TryMakeMove (Coord startSquare, Coord targetSquare) {
    int startIndex = BoardRepresentation::IndexFromCoord(startSquare);
    int targetIndex = BoardRepresentation::IndexFromCoord(targetSquare);
    bool moveIsLegal = false;
    Move *chosenMove;

    MoveGenerator moveGenerator;
    bool wantsKnightPromotion = QApplication::keyboardModifiers() & Qt::AltModifier;

    auto legalMoves = moveGenerator.GenerateMoves(board);
    for (Move& legalMove : legalMoves) {
        if (legalMove.StartSquare() == startIndex && legalMove.TargetSquare() == targetIndex) {
            if (legalMove.IsPromotion()) {
                if (legalMove.MoveFlag() == Move::Flag::PromoteToQueen && wantsKnightPromotion) {
                    continue;
                }
                if (legalMove.MoveFlag() != Move::Flag::PromoteToQueen && !wantsKnightPromotion) {
                    continue;
                }
            }
            moveIsLegal = true;
            chosenMove = &legalMove;
            break;
        }
    }

    if (moveIsLegal) {
        ChoseMove(*chosenMove);
        currentState = None;
    } else {
        CancelPieceSelection();
    }
}

