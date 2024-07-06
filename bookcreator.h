#ifndef BOOKCREATOR_H
#define BOOKCREATOR_H

#include "book.h"
#include <QElapsedTimer>
#include "board.h"

class BookCreator
{
public:
    int maxPlyToRecord;

    int minMovePlayCount = 10;

    QString gamesFilePath;
    QString bookFilePath;
    bool append;
    static Book loadBookFromFile(const QString &bookFilePath);
    BookCreator();
};

class PGNCreator {
public:
    static QString CreatePGN (QList<Move> moves);

    static QString NotationFromMove (QString currentFen, Move &move);

    static QString NotationFromMove (Board &board, Move &move);

    static QString GetSymbolFromPieceType (int pieceType);

};

#endif // BOOKCREATOR_H
