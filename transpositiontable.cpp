#include "transpositiontable.h"

TranspositionTable::TranspositionTable(Board *board, int size) {
    this->board = board;
    this->size = (uint64_t) size;
    entries.resize(size, nullptr);
    qDebug() << board->ZobristKey;
}

int TranspositionTable::LookupEvaluation (int depth, int plyFromRoot, int alpha, int beta) {
    if (!enabled) {
        return lookupFailed;
    }
    Entry entry = *entries[Index()];

    if (entry.key == board->ZobristKey) {
        // Only use stored evaluation if it has been searched to at least the same depth as would be searched now
        if (entry.depth >= depth) {
            int correctedScore = CorrectRetrievedMateScore (entry.value, plyFromRoot);
            // We have stored the exact evaluation for this position, so return it
            if (entry.nodeType == Exact) {
                return correctedScore;
            }
            // We have stored the upper bound of the eval for this position. If it's less than alpha then we don't need to
            // search the moves in this position as they won't interest us; otherwise we will have to search to find the exact value
            if (entry.nodeType == UpperBound && correctedScore <= alpha) {
                return correctedScore;
            }
            // We have stored the lower bound of the eval for this position. Only return if it causes a beta cut-off.
            if (entry.nodeType == LowerBound && correctedScore >= beta) {
                return correctedScore;
            }
        }
    }
    return lookupFailed;
}

void TranspositionTable::StoreEvaluation (int depth, int numPlySearched, int eval, int evalType, Move move) {
    if (!enabled) {
        return;
    }
    //uint64_t index = Index;
    //if (depth >= entries[Index].depth) {
    Entry* entry = new Entry (board->ZobristKey, CorrectMateScoreForStorage (eval, numPlySearched), (byte) depth, (byte) evalType, move);
    entries[Index()] = entry;
    //}
}
