#include "search.h"
#include "fennotation.h"

Search::Search(Board* board, AISettings* settings, QObject *parent)
    : QObject(parent),
    board(board),
    settings(settings),
    evaluation(),
    tt(board, transpositionTableSize),
    moveOrdering(moveGenerator, &tt),
    bestMoveThisIteration(Move::InvalidMove()),
    bestEvalThisIteration(0),
    bestMove(Move::InvalidMove()),
    bestEval(0),
    currentIterativeSearchDepth(0),
    abortSearch(false)
{
    // connect(settings, &AISettings::requestAbortSearch, this, &Search::endSearch);
}

void Search::startSearch() {
    initDebugInfo();

    // Initialize search settings
    bestEvalThisIteration = bestEval = 0;
    bestMoveThisIteration.moveValue = bestMove.moveValue = Move::InvalidMove().moveValue;
    tt.enabled = (settings->useTranspositionTable);

    if (settings->clearTTEachMove) {
        tt.Clear();
    }
    moveGenerator.promotionsToGenerate = (settings->promotionsToSearch);
    currentIterativeSearchDepth = 0;
    abortSearch = false;

    // Iterative deepening
    if (settings->useIterativeDeepening) {
        int targetDepth = settings->useFixedDepthSearch ? settings->depth : INT_MAX;

        for (int searchDepth = 1; searchDepth <= targetDepth; searchDepth++) {
            searchMoves(searchDepth, 0, negativeInfinity, positiveInfinity);
            if (abortSearch) {
                break;
            } else {
                currentIterativeSearchDepth = searchDepth;
                bestMove = bestMoveThisIteration;
                bestEval = bestEvalThisIteration;
                //update diagnostics
                searchDiagnostics.lastCompletedDepth = searchDepth;
                searchDiagnostics.move = bestMove.Name();
                searchDiagnostics.eval = bestEval;
                // searchDiagnostics.moveVal = Chess.PGNCreator.NotationFromMove (FenNotation::CurrentFen (*board), bestMove);-----------------------------notation Value

                // Exit search if found a mate
                if (isMateScore(bestEval) && !settings->endlessSearchMode) {
                    break;
                }
            }
        }
    } else {
        searchMoves(settings->depth, 0, negativeInfinity, positiveInfinity);
        bestMove.moveValue = bestMoveThisIteration.moveValue;
        bestEval = bestEvalThisIteration;
    }

    emit onSearchComplete(bestMove);

    if (!settings->useThreading) {
        logDebugInfo();
    }
}

std::pair<Move, int> Search::getSearchResult() {
    return {bestMove, bestEval};
}

void Search::endSearch() {
    abortSearch = true;
}

void Search::initDebugInfo() {
    searchTimer.start();
    numNodes = 0;
    numQNodes = 0;
    numCutoffs = 0;
    numTranspositions = 0;
}

int Search::searchMoves(int depth, int plyFromRoot, int alpha, int beta) {
    qDebug() << "depth: " << depth ;
    if (abortSearch) {
        return 0;
    }

    if (plyFromRoot > 0) {
        // Detect draw by repetition.
        // Returns a draw score even if this position has only appeared once in the game history (for simplicity).
        if (board->RepetitionPositionHistory.contains(board->ZobristKey)) {
            return 0;
        }

        // Skip this position if a mating sequence has already been found earlier in
        // the search, which would be shorter than any mate we could find from here.
        // This is done by observing that alpha can't possibly be worse (and likewise
        // beta can't  possibly be better) than being mated in the current position.
        alpha = qMax (alpha, -immediateMateScore + plyFromRoot);
        beta = qMin (beta, immediateMateScore - plyFromRoot);
        if (alpha >= beta) {
            return alpha;
        }
    }
    // Try looking up the current position in the transposition table.
    // If the same position has already been searched to at least an equal depth
    // to the search we're doing now,we can just use the recorded evaluation.

    // int ttVal = tt.LookupEvaluation (depth, plyFromRoot, alpha, beta);
    // if (ttVal != TranspositionTable::lookupFailed) {
    //     numTranspositions++;
    //     if (plyFromRoot == 0) {
    //         bestMoveThisIteration = tt.GetStoredMove ();
    //         bestEvalThisIteration = tt.entries[tt.Index()]->value;
    //         //Debug.Log ("move retrieved " + bestMoveThisIteration.Name + " Node type: " + tt.entries[tt.Index].nodeType + " depth: " + tt.entries[tt.Index].depth);
    //     }
    //     return ttVal;
    // }

    if (depth == 0) {
        int evaluation = quiescenceSearch (alpha, beta);
        return evaluation;
    }
    QList<Move> moves = moveGenerator.GenerateMoves (board);
    qDebug() << moves.size();
    moveOrdering.OrderMoves (*board, moves, settings->useTranspositionTable);
    // Detect checkmate and stalemate when no legal moves are available
    if (moves.size() == 0) {
        if (moveGenerator.InCheck ()) {
            int mateScore = immediateMateScore - plyFromRoot;
            return -mateScore;
        } else {
            return 0;
        }
    }

    int evalType = TranspositionTable::UpperBound;
    Move bestMoveInThisPosition = invalidMove;

    for (int i = 0; i < moves.size(); i++) {
        board->makeMove (moves[i], true);
        int eval = -searchMoves (depth - 1, plyFromRoot + 1, -beta, -alpha);
        board->unmakeMove (moves[i], true);
        numNodes++;

        // Move was *too* good, so opponent won't allow this position to be reached
        // (by choosing a different move earlier on). Skip remaining moves.
        if (eval >= beta) {
            tt.StoreEvaluation (depth, plyFromRoot, beta, TranspositionTable::LowerBound, moves[i]);
            numCutoffs++;
            return beta;
        }

        // Found a new best move in this position
        if (eval > alpha) {
            evalType = TranspositionTable::Exact;
            bestMoveInThisPosition = moves[i];

            alpha = eval;
            if (plyFromRoot == 0) {
                bestMoveThisIteration = moves[i];
                bestEvalThisIteration = eval;
            }
        }
    }

    tt.StoreEvaluation (depth, plyFromRoot, alpha, evalType, bestMoveInThisPosition);

    return alpha;
}

// Search capture moves until a 'quiet' position is reached.
int Search::quiescenceSearch(int alpha, int beta) {
    // A player isn't forced to make a capture (typically), so see what the evaluation is without capturing anything.
    // This prevents situations where a player ony has bad captures available from being evaluated as bad,
    // when the player might have good non-capture moves available.
    int eval = evaluation.Evaluate (*board);
    searchDiagnostics.numPositionsEvaluated++;
    if (eval >= beta) {
        return beta;
    }
    if (eval > alpha) {
        alpha = eval;
    }

    auto moves = moveGenerator.GenerateMoves (board, false);
    moveOrdering.OrderMoves (*board, moves, false);
    for (int i = 0; i < moves.size(); i++) {
        board->makeMove (moves[i], true);
        eval = -quiescenceSearch (-beta, -alpha);
        board->unmakeMove (moves[i], true);
        numQNodes++;

        if (eval >= beta) {
            numCutoffs++;
            return beta;
        }
        if (eval > alpha) {
            alpha = eval;
        }
    }

    return alpha;
}


bool Search::isMateScore(int score){
    const int maxMateDepth = 1000;
    return std::abs(score) > immediateMateScore - maxMateDepth;
}

int Search::numPlyToMateFromScore(int score) {
    return immediateMateScore - std::abs(score);
}

void Search::logDebugInfo() {
    announceMate();
    qDebug() << "Best move:" << bestMoveThisIteration.Name() << "Eval:" << bestEvalThisIteration << "Search time:" << searchTimer.elapsed() << "ms.";
    qDebug() << "Num nodes:" << numNodes << "num Qnodes:" << numQNodes << "num cutoffs:" << numCutoffs << "num TThits" << numTranspositions;
}

void Search::announceMate() {
    if (isMateScore(bestEvalThisIteration)) {
        int numPlyToMate = numPlyToMateFromScore(bestEvalThisIteration);
        int numMovesToMate = static_cast<int>(std::ceil(numPlyToMate / 2.0));
        QString sideWithMate = (bestEvalThisIteration * (board->WhiteToMove ? 1 : -1) < 0) ? "Black" : "White";
        qDebug() << sideWithMate << "can mate in" << numMovesToMate << "move" << (numMovesToMate > 1 ? "s" : "");
    }
}
