#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include <QVector>
#include <QElapsedTimer>
#include "board.h"
#include "move.h"
#include "moveGenerator.h"
#include "evaluation.h"
#include "moveordering.h"
#include "transpositiontable.h"
#include "aisettings.h"
#include "searchdiagnostics.h"

class Search : public QObject
{
    Q_OBJECT

public:
    explicit Search(Board* board, AISettings* settings, QObject *parent = nullptr);

    static const int transpositionTableSize = 64000;
    static const int immediateMateScore = 100000;
    static const int positiveInfinity = 9999999;
    static const int negativeInfinity = -positiveInfinity;

    Board* board;
    void startSearch();
    std::pair<Move, int> getSearchResult();
    void endSearch();
    static bool isMateScore(int score);
    SearchDiagnostics searchDiagnostics;
    static int numPlyToMateFromScore(int score) ;

signals:
    void onSearchComplete(const Move bestMove);

private:
    void initDebugInfo();
    int searchMoves(int depth, int plyFromRoot, int alpha, int beta);
    int quiescenceSearch(int alpha, int beta);


    void logDebugInfo();
    void announceMate();

    AISettings* settings;
    Evaluation evaluation;
    MoveGenerator moveGenerator;
    TranspositionTable tt;
    MoveOrdering moveOrdering;

    Move bestMoveThisIteration;
    int bestEvalThisIteration;
    Move bestMove;
    int bestEval;
    int currentIterativeSearchDepth;
    bool abortSearch;

    Move invalidMove = Move::InvalidMove();

    // Diagnostics
    int numNodes;
    int numQNodes;
    int numCutoffs;
    int numTranspositions;
    QElapsedTimer searchTimer;
};

#endif // SEARCH_H
