#ifndef BOOK_H
#define BOOK_H
#include <QMap>
#include <QList>
#include <QRandomGenerator>
#include "move.h"
class Book
{
public:
    class BookPosition {
    public:
        QMap<ushort, int> numTimesMovePlayed;

        void AddMove (Move &move, int numTimesPlayed = 1) {
            ushort moveValue = move.Value();

            if (numTimesMovePlayed.contains (moveValue)) {
                numTimesMovePlayed[moveValue]++;
            } else {
                numTimesMovePlayed.insert(moveValue, numTimesPlayed);
            }
        }
    };
    QMap<uint64_t, BookPosition> bookPositions;
    Book();
    bool HasPosition (uint64_t positionKey) {
        return bookPositions.contains(positionKey);
    }

    BookPosition GetBookPosition (uint64_t key) {
        return bookPositions[key];
    }

    Move GetRandomBookMove (uint64_t key);
    Move GetRandomBookMoveWeighted (uint64_t key);

    void Add (uint64_t positionKey, Move move);
    void Add (uint64_t positionKey, Move move, int numTimesPlayed);

private:
    void SmoothWeights (QList<float> &weights, float strength = 0.1f);
};

#endif // BOOK_H
