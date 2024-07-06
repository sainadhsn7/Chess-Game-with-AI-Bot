#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include <cstdint>
#include "board.h"

typedef uint8_t byte;


class TranspositionTable
{
public:
    struct Entry {
        uint64_t key;
        int value;
        Move move;
        byte depth;
        byte nodeType;

        //public readonly byte gamePly;
        Entry (uint64_t key, int value, byte depth, byte nodeType, Move move):move(move) {
            this->key = key;
            this->value = value;
            this->depth = depth; // depth is how many ply were searched ahead from this position
            this->nodeType = nodeType;
        }
        static int GetSize () {
            return sizeof(Entry);
        }
    };
    static const int lookupFailed = INT32_MIN;

    // The value for this position is the exact evaluation
    static const int Exact = 0;
    // A move was found during the search that was too good, meaning the opponent will play a different move earlier on,
    // not allowing the position where this move was available to be reached. Because the search cuts off at
    // this point (beta cut-off), an even better move may exist. This means that the evaluation for the
    // position could be even higher, making the stored value the lower bound of the actual value.
    static const int LowerBound = 1;
    // No move during the search resulted in a position that was better than the current player could get from playing a
    // different move in an earlier position (i.e eval was <= alpha for all moves in the position).
    // Due to the way alpha-beta search works, the value we get here won't be the exact evaluation of the position,
    // but rather the upper bound of the evaluation. This means that the evaluation is, at most, equal to this value.
    static const int UpperBound = 2;

    QList<Entry*> entries;

    uint64_t size;
    bool enabled = true;
    Board *board;

    TranspositionTable (Board *board, int size);

    void Clear () {
        for (int i = 0; i < entries.size(); i++) {
            if(entries[i]){
                delete entries[i];
                entries[i] = nullptr;
            }
        }
    }

    uint64_t Index() {
        return (board->ZobristKey % size);
    }

    Move GetStoredMove () {
        if(Index() >= uint64_t(entries.size())){
            qDebug() << "TT index out of bounds";
            return Move(0);
        }
        return entries[Index()]->move;
    }

    int LookupEvaluation (int depth, int plyFromRoot, int alpha, int beta);

    void StoreEvaluation (int depth, int numPlySearched, int eval, int evalType, Move move);

    int CorrectMateScoreForStorage (int score, int numPlySearched) {
        if (isMateScore (score)) {
            int sign = (score>0)-(score<0);
            return (score * sign + numPlySearched) * sign;
        }
        return score;
    }

    int CorrectRetrievedMateScore (int score, int numPlySearched) {
        if (isMateScore (score)) {
            int sign = (score>0)-(score<0);
            return (score * sign - numPlySearched) * sign;
        }
        return score;
    }
private:
    bool isMateScore(int score){
        const int maxMateDepth = 1000;
        return std::abs(score) > 100000 - maxMateDepth;
    }
};

#endif // TRANSPOSITIONTABLE_H
