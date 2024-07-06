#include "gamemanager.h"
#include "aiplayer.h"
#include <QCoreApplication>
#include <QClipboard>
#include <QApplication>
#include <QTimer>
#include <QUrl>
#include <QDesktopServices>
#include "bookcreator.h"
#include "humanplayer.h"
#include "mainwindow.h"
#include "qdatetime.h"

GameManager::GameManager(MainWindow *parWindow, QObject *parent)
    : QObject(parent),loadCustomPosition(false), customPosition("1rbq1r1k/2pp2pp/p1n3p1/2b1p3/R3P3/1BP2N2/1P3PPP/1NBQ1RK1 w - - 0 1")
{
    whitePlayerType = Human;
    blackPlayerType = Human;

    // aiSettings = new AISettings;
    board = new Board;
    searchBoard = new Board;
    boardUI = new BoardUI(parWindow);
    updateTimer = new QTimer(this);

    playerToMove = nullptr;
    whitePlayer = nullptr;
    blackPlayer = nullptr;

    //if(useclocks)
    //whiteClock = new Clock, blackClock = new Clock;
    this->parWindow = parWindow;
    connect(updateTimer, &QTimer::timeout, this, &GameManager::update);
}

GameManager::~GameManager()
{
    delete board;
    delete boardUI;
    delete searchBoard;
    // delete aiSettings;
    delete updateTimer;
    playerToMove = nullptr;
    if(whitePlayer)delete whitePlayer;
    if(blackPlayer)delete blackPlayer;
    //if(useclocks)
    // delete whiteClock;
    // delete blackClock;
}

void GameManager::start(){
    newGame(whitePlayerType, blackPlayerType);
    updateTimer->start(50); // Approximately 60 FPS
}
void GameManager::update () {
    updateTimer->stop();
    auto startTime = QTime::currentTime();
    zobristDebug = board->ZobristKey;
    if (gameResult == Playing) {
        logAIDiagnostics ();
        bool temp=0;
        if(dynamic_cast<AIPlayer*> (playerToMove) != nullptr){
            qDebug() << "AIPlayer's Turn";
            temp=1;
            // return;
        }
        playerToMove->Update ();
        // qDebug() << "return from " << (temp?"AI ":"Human ")  << "player update function";
        // if (useClocks) {
        //     whiteClock.isTurnToMove = board.WhiteToMove;
        //     blackClock.isTurnToMove = !board.WhiteToMove;
        // }
    }

    // Example for exporting the game (could be replaced with a more appropriate trigger in Qt)
    // if (QApplication::keyboardModifiers() & Qt::ControlModifier && QApplication::keyDown(Qt::Key_E)) {
    //     exportGame();
    // }
    auto elapsed = startTime.msecsTo(QTime::currentTime());
    int nextInterval = qMax(0, 16-elapsed);
    updateTimer->start(nextInterval);
}
void GameManager:: onMoveChosen(Move &move){
    bool animateMove = dynamic_cast<AIPlayer*> (playerToMove) != nullptr;
    board->makeMove (move);
    searchBoard->makeMove (move);
    gameMoves.push_back(move);
    // emit moveMade(move);
    boardUI->OnMoveMade (board, move, animateMove);
    notifyPlayerToMove ();
}


void GameManager::notifyPlayerToMove()
{
    gameResult = getGameState();
    printGameResult(gameResult);
    if (gameResult == Playing) {
        playerToMove = board->WhiteToMove ? whitePlayer : blackPlayer;
        if(!board->WhiteToMove){
            qDebug() << "notify aiplayer";
        }
        else{
            qDebug() << "notify human";
        }
        playerToMove->NotifyTurnToMove();
    } else {
        qDebug() << "Game Over";
    }
}

void GameManager::newGame (PlayerType whitePlayerType, PlayerType blackPlayerType) {
    gameMoves.clear ();
    if (loadCustomPosition) {
        board->loadPosition (customPosition);
        searchBoard->loadPosition (customPosition);
    } else {
        board->loadStartPosition ();
        searchBoard->loadStartPosition ();
    }
    // emit positionLoaded();
    boardUI->updatePosition (board);
    boardUI->ResetSquareColor();

    createPlayer ( whitePlayer, whitePlayerType);
    createPlayer ( blackPlayer, blackPlayerType);

    gameResult = Playing;
    printGameResult (gameResult);
    notifyPlayerToMove ();
}

void GameManager::newGame (bool humanPlaysWhite) {
    boardUI->setPerspective (humanPlaysWhite);
    newGame ((humanPlaysWhite) ? Human : AI, (humanPlaysWhite) ? AI : Human);
}

GameManager::Result GameManager::getGameState()
{
    MoveGenerator moveGenerator;
    auto moves = moveGenerator.GenerateMoves(board);
    qDebug() << FenNotation::CurrentFen(board);
    qDebug() << "moves size() "<< moves.size();
    // printBoard();
    // Look for mate/stalemate
    if (moves.isEmpty()) {
        if (moveGenerator.InCheck()) {
            return board->WhiteToMove ? WhiteIsMated : BlackIsMated;
        }
        return Stalemate;
    }

    // Fifty move rule
    if (board->fiftyMoveCounter >= 100) {
        return FiftyMoveRule;
    }

    // Threefold repetition
    int repCount = board->RepetitionPositionHistory.count(board->ZobristKey);
    if (repCount == 3) {
        return Repetition;
    }

    // Look for insufficient material (not all cases implemented yet)
    int numPawns = board->pawns[Board::WhiteIndex].Count() + board->pawns[Board::BlackIndex].Count();
    int numRooks = board->rooks[Board::WhiteIndex].Count() + board->rooks[Board::BlackIndex].Count();
    int numQueens = board->queens[Board::WhiteIndex].Count() + board->queens[Board::BlackIndex].Count();
    int numKnights = board->knights[Board::WhiteIndex].Count() + board->knights[Board::BlackIndex].Count();
    int numBishops = board->bishops[Board::WhiteIndex].Count() + board->bishops[Board::BlackIndex].Count();

    if (numPawns + numRooks + numQueens == 0) {
        if (numKnights == 1 || numBishops == 1) {
            return InsufficientMaterial;
        }
    }

    return Playing;
}

void GameManager::createPlayer(Player *&player, PlayerType playerType)
{
    if (player) {
        disconnect(player, &Player::moveChosen, this, &GameManager::onMoveChosen);
        delete player;
        player = nullptr;
    }

    if (playerType == Human) {
        player = new HumanPlayer(board, boardUI);
    } else {
        player = new AIPlayer(searchBoard, aiSettings);
    }
    connect(player, &Player::moveChosen, this, &GameManager::onMoveChosen, Qt::DirectConnection);
}

void GameManager::logAIDiagnostics()
{
    QString text;
    auto d = &aiSettings->diagnostics;
    text += QString("<color=#%1>Version 1.0\n").arg(colors[3].name().mid(1));
    text += QString("<color=#%1>Depth Searched: %2").arg(colors[0].name().mid(1)).arg(d->lastCompletedDepth);

    QString evalString;
    if (d->isBook) {
        evalString = "Book";
    } else {
        float displayEval = d->eval / 100.0f;
        if (dynamic_cast<AIPlayer*>(playerToMove) && !board->WhiteToMove) {
            displayEval = -displayEval;
        }
        evalString = QString("%1").arg(displayEval, 0, 'f', 2).replace(",", ".");
        if (Search::isMateScore(d->eval)) {
            evalString = QString("mate in %1 ply").arg(Search::numPlyToMateFromScore(d->eval));
        }
    }
    text += QString("\n<color=#%1>Eval: %2").arg(colors[1].name().mid(1)).arg(evalString);
    text += QString("\n<color=#%1>Move: %2").arg(colors[2].name().mid(1)).arg(d->moveVal);
    // aiDiagnosticsUI->setText(text);
}

void GameManager::NewComputerVersusComputerGame () {
    boardUI->setPerspective (true);
    newGame (AI, AI);
}

void GameManager::exportGame()
{
    QString pgn = PGNCreator::CreatePGN(gameMoves.toList());
    QString baseUrl = "https://www.lichess.org/paste?pgn=";
    QString escapedPGN = QUrl::toPercentEncoding(pgn);
    QString url = baseUrl + escapedPGN;

    QDesktopServices::openUrl(QUrl(url));

    QApplication::clipboard()->setText(pgn);
}

void GameManager::quitGame()
{
    QCoreApplication::quit();
}

void GameManager::printGameResult(Result result)
{
    // float subtitleSize = resultUI->fontSize() * 0.75f;
    // QString subtitleSettings = QString("<color=#787878> <size=%1>").arg(subtitleSize);

    // switch (result) {
    // case Playing:
    //     resultUI->setText("");
    //     break;
    // case WhiteIsMated:
    // case BlackIsMated:
    //     resultUI->setText("Checkmate!");
    //     break;
    // case FiftyMoveRule:
    //     resultUI->setText("Draw");
    //     resultUI->append(subtitleSettings + "\n(50 move rule)");
    //     break;
    // case Repetition:
    //     resultUI->setText("Draw");
    //     resultUI->append(subtitleSettings + "\n(3-fold repetition)");
    //     break;
    // case Stalemate:
    //     resultUI->setText("Draw");
    //     resultUI->append(subtitleSettings + "\n(Stalemate)");
    //     break;
    // case InsufficientMaterial:
    //     resultUI->setText("Draw");
    //     resultUI->append(subtitleSettings + "\n(Insufficient material)");
    //     break;
    // }
}
