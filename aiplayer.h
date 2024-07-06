#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "Player.h"
#include "search.h"
#include "book.h"

class AIPlayer: public Player
{
    Q_OBJECT
public:
    AIPlayer(Board *board, AISettings *settings, QObject *parent = nullptr);
    ~AIPlayer() override;
    void Update() override;
    void NotifyTurnToMove() override;

private slots:
    void startSearch();
    void startThreadedSearch();
    void timeOutThreadedSearch();
    void playBookMove(Move bookMove);
    void onSearchComplete(Move move);

private:
    const int bookMoveDelayMillis = 250;
    Search *search;
    AISettings *settings;
    bool moveFound;
    Move *move;
    Board *board;
    QTimer *cancelSearchTimer;
    Book *book;
};

#endif // AIPLAYER_H
