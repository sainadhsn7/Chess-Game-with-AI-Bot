#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include "move.h"
#include <QDebug>
#include <QThread>
class Player : public QObject {
    Q_OBJECT

public:
    explicit Player(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Player() = default;

    // Pure virtual methods (abstract methods)
    virtual void Update() = 0;
    virtual void NotifyTurnToMove() = 0;

protected:
    // Signal to notify when a move is chosen
    virtual void ChoseMove(Move& move) {
        emit moveChosen(move);
    }

signals:
    void moveChosen(Move& move);
};

#endif // PLAYER_H
