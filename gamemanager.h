#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include "board.h"
#include "Player.h"
#include "aisettings.h"
#include "boardui.h"

class MainWindow;

class GameManager: public QObject
{
    Q_OBJECT
public:
    enum Result { Playing, WhiteIsMated, BlackIsMated, Stalemate, Repetition, FiftyMoveRule, InsufficientMaterial };
    enum PlayerType { Human, AI };

    bool loadCustomPosition;
    QString customPosition;

    PlayerType whitePlayerType;
    PlayerType blackPlayerType;
    AISettings *aiSettings;
    QColor colors[3];
    ulong zobristDebug;

    //bool useClocks;
    // Clock *whiteClock;
    // Clock *blackClock;
    explicit GameManager(MainWindow *parWindow, QObject *parent = nullptr);
    ~GameManager();

    void start();
    void newGame(bool humanPlaysWhite);
    void newGame (PlayerType whitePlayerType, PlayerType blackPlayerType);
    void NewComputerVersusComputerGame();
    void exportGame();
    void quitGame();


signals:
    void positionLoaded();
    void moveMade(Move &move);

public slots:
    void onMoveChosen(Move &move);
    void update();

private:
    MainWindow *parWindow;
    void createPlayer(Player *&player, PlayerType playerType);
    void notifyPlayerToMove();
    void logAIDiagnostics();
    void printGameResult(Result result);
    void printBoard();
    Result getGameState();

    Board *board;
    Board *searchBoard;
    BoardUI *boardUI;
    QTimer *updateTimer;
    QList<Move> gameMoves;
    Player *playerToMove;
    Result gameResult;
    Player *whitePlayer;
    Player *blackPlayer;

};

#endif // GAMEMANAGER_H
