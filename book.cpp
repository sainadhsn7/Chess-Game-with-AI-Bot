#include "book.h"

Book::Book() {}

Move Book::GetRandomBookMove (uint64_t key) {
    auto p = bookPositions[key];
    // auto moves =  QList<ushort> (p.numTimesMovePlayed.Keys).ToArray ();
    QList<ushort> moves;
    for(auto it = p.numTimesMovePlayed.begin(); it!=p.numTimesMovePlayed.end(); it++){
        moves.push_back(it.key());
    }
    auto prng = QRandomGenerator::global();
    ushort randomMove = moves[prng->bounded(0, moves.size())];
    return Move(randomMove);
}

Move Book::GetRandomBookMoveWeighted (uint64_t key) {
    auto p = bookPositions[key];
    QList<ushort> moves;
    for(auto it = p.numTimesMovePlayed.begin(); it!=p.numTimesMovePlayed.end(); it++){
        moves.push_back(it.key());
    }
    auto numTimesMovePlayed = p.numTimesMovePlayed.values();

    QList<float> moveWeights(moves.length());
    for (int i = 0; i < moves.length(); i++) {
        moveWeights[i] = numTimesMovePlayed[i];
    }

    // Smooth weights to increase probability of rarer moves
    // (strength of 1 would make all moves equally likely)
    SmoothWeights (moveWeights, 0.5f);

    float sum = 0;
    for (int i = 0; i < moves.length(); i++) {
        sum += moveWeights[i];
    }

    float moveProbabilitiesCumul[moves.length()];
    float previousProbability = 0;
    for (int i = 0; i < moves.length(); i++) {
        moveProbabilitiesCumul[i] = previousProbability + moveWeights[i] / sum;
        previousProbability = moveProbabilitiesCumul[i];
    }

    auto prng = QRandomGenerator::global();
    float t = (float) prng->generateDouble();

    //for (int i = 0; i < moves.Length; i++) {
    //Debug.Log ((new Move (moves[i]).Name) + "  " + moveProbabilitiesCumul[i] + "  " + t);
    //}

    for (int i = 0; i < moves.length(); i++) {
        if (t <= moveProbabilitiesCumul[i]) {
            return Move (moves[i]);
        }
    }

    return Move(moves[0]);
}

void Book::SmoothWeights (QList<float> &weights, float strength) {
    float sum = 0;
    for (int i = 0; i < weights.length(); i++) {
        sum += weights[i];
    }
    float avg = sum / weights.length();

    for (int i = 0; i < weights.length(); i++) {
        float offsetFromAvg = avg - weights[i];
        weights[i] += offsetFromAvg * strength;
    }
}

void Book::Add (uint64_t positionKey, Move move) {
    if (!bookPositions.contains(positionKey)) {
        bookPositions.insert (positionKey, BookPosition());
    }

    bookPositions[positionKey].AddMove(move);
}

void Book::Add (uint64_t positionKey, Move move, int numTimesPlayed) {
    if (!bookPositions.contains(positionKey)) {
        bookPositions.insert(positionKey,  BookPosition ());
    }

    bookPositions[positionKey].AddMove (move, numTimesPlayed);
}
