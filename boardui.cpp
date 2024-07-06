#include "boardui.h"
#include "mainwindow.h"
#include <QDebug>
#include <QPropertyAnimation>
#include <QTimer>

typedef BoardRepresentation BR;

BoardUI::~BoardUI(){
    for(int i=0; i<64; i++){
        if(Square[i]){
            scene->removeItem(Square[i]);
            delete Square[i];
            Square[i] = nullptr;
        }
        if(SquarePiece[i]){
            scene->removeItem(SquarePiece[i]->UIPiece);
            delete SquarePiece[i];
            SquarePiece[i] = nullptr;
        }
    }
    delete view;
    delete scene;
}

void BoardUI::CreateBoardUI(){

    view = new QGraphicsView(parWindow);
    parWindow->setCentralWidget(view);

    scene = new QGraphicsScene(parWindow);
    view->setScene(scene);

    boardLen = 8 * squareLen;
    view->setFixedSize(boardLen+8, boardLen+8);
    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            QColor color = (rank + file) % 2 == 1 ? boardTheme.lightSquares.normal : boardTheme.darkSquares.normal;//a0(0, 0) is black
            auto pos = PositionFromCoord(file, rank);
            Square[rank * 8 + file] = scene->addRect(pos.x(), pos.y(), squareLen, squareLen, QPen(Qt::black), QBrush(color));
        }
    }

    QSize windowSize = view->sizeHint() + QSize(2 * view->frameWidth(), 2 * view->frameWidth());
    parWindow->setFixedSize(windowSize);

    ResetSquareColor();
}

void BoardUI::updatePosition(Board* board){
    for(int rank = 0; rank < 8; rank++){
        for(int file = 0; file < 8; file ++){
            const int square = BR::IndexFromCoord(Coord(rank, file));
            if(SquarePiece[square]){
                auto PieceItem = SquarePiece[square];
                scene->removeItem(PieceItem->UIPiece);
                SquarePiece[square] = nullptr;
                delete PieceItem;
            }
            if(board->Square[square] == Piece::None)continue;
            SquarePiece[square] = new PieceUI(board->Square[square], squareLen);
            SquarePiece[square]->SetPosition(PositionFromCoord(file, rank));
            scene->addItem(SquarePiece[square]->UIPiece);
        }
    }
}

void BoardUI::HighlightLegalMoves (Board *board, Coord fromSquare){
    if(showLegalMoves){
        auto moves = moveGenerator.GenerateMoves (board);
        for (int i = 0; i < moves.size(); i++) {
            Move &move = moves[i];
            if (move.StartSquare() == BR::IndexFromCoord (fromSquare)) {
                Coord coord = BR::CoordFromIndex(move.TargetSquare());
                setSquareColor(coord, boardTheme.lightSquares.legal, boardTheme.darkSquares.legal);
            }
        }
    }
}

void BoardUI::selectSquare(Coord coord){
    setSquareColor(coord, boardTheme.lightSquares.selected, boardTheme.darkSquares.selected);
}

void BoardUI::deselectSquare(Coord coord){
    ResetSquareColor();
}

void BoardUI::HighlightMove(Move &move){
    setSquareColor(BR::CoordFromIndex(move.StartSquare()), boardTheme.lightSquares.moveFromHighlight, boardTheme.darkSquares.moveFromHighlight);
    setSquareColor(BR::CoordFromIndex(move.TargetSquare()), boardTheme.lightSquares.moveToHighlight, boardTheme.darkSquares.moveToHighlight);
}

void BoardUI::setPerspective(bool whitePov){
    whiteIsBottom = whitePov;
    ResetSquarePosition();
}

void BoardUI::ResetSquarePosition(){

    for(int rank = 0; rank < 8; rank ++){
        for(int file = 0; file < 8; file ++){
            int square = BR::IndexFromCoord(file, rank);
            Square[square]->setPos(PositionFromCoord(file, rank));
            SquarePiece[square]->SetPosition(PositionFromCoord(file, rank));
        }
    }
    if(!lastMadeMove.IsInvalid() and Highlight){
        HighlightMove(lastMadeMove);
    }
}
void BoardUI::ResetSquareColor(){
    for(int rank = 0; rank < 8; rank ++){
        for(int file = 0; file < 8; file ++){
            setSquareColor(Coord(rank, file), boardTheme.lightSquares.normal, boardTheme.darkSquares.normal);
        }
    }
    if(!lastMadeMove.IsInvalid() and Highlight){
        HighlightMove(lastMadeMove);
    }
}

void BoardUI::setSquareColor(Coord square, QColor light, QColor dark){
    Square[square.rankIndex * 8 + square.fileIndex]->setBrush(square.IsLightSquare()? light:dark);
}

void BoardUI::DragPiece(Coord pieceCoord, QPointF mousePos) {
    int index = BR::IndexFromCoord(pieceCoord);
    if (SquarePiece[index]) {
        mousePos = QPointF(mousePos.x() - 40, mousePos.y() - 40);
        SquarePiece[index]->SetPosition(mousePos);
        SquarePiece[index]->UIPiece->setZValue(pieceDragDepth); // Adjust z value for dragging
    }
}

void BoardUI::ResetPiecePosition(Coord pieceCoord) {
    int square = BR::IndexFromCoord(pieceCoord);
    if(SquarePiece[square]){
        SquarePiece[square]->SetPosition(PositionFromCoord(pieceCoord));
    }
}

bool BoardUI::TryGetSquareUnderMouse(QPointF mouseWorld, Coord &selectedCoord) {
    int file = static_cast<int>(mouseWorld.x()/squareLen);
    int rank = static_cast<int>(mouseWorld.y()/squareLen);
    if (whiteIsBottom) {
        rank = 7 - rank;
    }
    else{
        file = 7 - file;
    }
    selectedCoord = Coord(rank, file);
    return (file >= 0 && file < 8 && rank >= 0 && rank < 8);
}

void BoardUI::OnMoveMade(Board *board, Move &move, bool animate) {
    lastMadeMove = move;
    if (animate) {
        AnimateMove(move, board);
    } else {
        updatePosition(board);
        ResetSquareColor();
    }
}

void BoardUI::AnimateMove(Move &move, Board *board) {
    int startSquare = move.StartSquare();
    int targetSquare = move.TargetSquare();

    Coord startCoord = BR::CoordFromIndex(startSquare);
    Coord targetCoord = BR::CoordFromIndex(targetSquare);

    auto* piece = SquarePiece[startSquare]->UIPiece; // Assuming SquarePiece holds QGraphicsRectItem or similar
    QPointF startPos = PositionFromCoord(startCoord);
    QPointF targetPos = PositionFromCoord(targetCoord);

    qDebug() << startCoord.fileIndex << " " << startCoord.rankIndex << "\n";
    qDebug() << targetCoord.fileIndex << " " << targetCoord.rankIndex << "\n";

    qDebug() << startPos.x() << " " << startPos.y() << "\n";
    qDebug() << targetPos.x() << " " << targetPos.y() << "\n";

    QPropertyAnimation* animation = new QPropertyAnimation(piece, "pos");
    animation->setDuration(1500); // duration in milliseconds
    animation->setStartValue(startPos);
    animation->setEndValue(targetPos);
    QAbstractAnimation::connect(animation, &QPropertyAnimation::finished, [this, board, startPos, targetSquare]() {
        updatePosition(board);
        ResetSquareColor();
        // qDebug() << SquarePiece[startSquare]->PieceVal << "\n";
        SquarePiece[targetSquare]->UIPiece->setPos(startPos);// ------------------------------- make the piece to go back original position
        // piece->setPos(startPos); // Resetting position to startPos
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

QPointF BoardUI::PositionFromCoord(int file, int rank)  {
    if(whiteIsBottom){
        return QPointF(file*squareLen, (7-rank)*squareLen);
    }
    return QPointF((7-file)*squareLen, rank*squareLen);
}

QPointF BoardUI::PositionFromCoord(Coord coord) {
    return PositionFromCoord(coord.fileIndex, coord.rankIndex);
}
