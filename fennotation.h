#ifndef FENNOTATION_H
#define FENNOTATION_H

#include <QMap>
#include "piece.h"


class Board;
class FenNotation
{
public:
    class LoadedPositionInfo{
    public:
        int squares[64];
        bool whiteCastleKingside;
        bool whiteCastleQueenside;
        bool blackCastleKingside;
        bool blackCastleQueenside;
        int epFile;
        bool whiteToMove;
        int plyCount;

        LoadedPositionInfo () {
            memset(squares, 0, sizeof(int)*64);
        }
    };
public:
    static const QMap<QChar, int> pieceTypeFromSymbol;
    static const QString startFen;
    FenNotation();
    static LoadedPositionInfo PositionFromFen(QString fen);
    static QString CurrentFen(Board *board);
};

#endif // FENNOTATION_H
