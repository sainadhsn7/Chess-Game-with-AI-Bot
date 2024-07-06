#include "moveordering.h"

MoveOrdering::MoveOrdering (MoveGenerator moveGenerator, TranspositionTable *tt) {
    moveScores.resize(maxMoveCount);
    this->moveGenerator = moveGenerator;
    this->transpositionTable = tt;
}

MoveOrdering::~MoveOrdering(){
}
void MoveOrdering::OrderMoves (Board board, QList<Move> moves, bool useTT) {
    Move hashMove(0);
    if (useTT) {
        hashMove.moveValue = transpositionTable->GetStoredMove ().moveValue;
    }

    for (int i = 0; i < moves.size(); i++) {
        int score = 0;
        int movePieceType = Piece::PieceType (board.Square[moves[i].StartSquare()]);
        int capturePieceType = Piece::PieceType (board.Square[moves[i].TargetSquare()]);
        int flag = moves[i].MoveFlag();

        if (capturePieceType != Piece::None) {
            // Order moves to try capturing the most valuable opponent piece with least valuable of own pieces first
            // The capturedPieceValueMultiplier is used to make even 'bad' captures like QxP rank above non-captures
            score = capturedPieceValueMultiplier * GetPieceValue (capturePieceType) - GetPieceValue (movePieceType);
        }

        if (movePieceType == Piece::Pawn) {

            if (flag == Move::Flag::PromoteToQueen) {
                score += Evaluation::queenValue;
            } else if (flag == Move::Flag::PromoteToKnight) {
                score += Evaluation::knightValue;
            } else if (flag == Move::Flag::PromoteToRook) {
                score += Evaluation::rookValue;
            } else if (flag == Move::Flag::PromoteToBishop) {
                score += Evaluation::bishopValue;
            }
        } else {
            // Penalize moving piece to a square attacked by opponent pawn
            if (BitBoard::ContainsSquare (moveGenerator.opponentPawnAttackMap, moves[i].TargetSquare())) {
                score -= squareControlledByOpponentPawnPenalty;
            }
        }
        if (Move::SameMove (moves[i], hashMove)) {
            score += 10000;
        }

        moveScores[i] = score;
    }

    Sort (moves);
}

void MoveOrdering::Sort (QList<Move> moves) {
    // Sort the moves list based on scores
    for (int i = 0; i < moves.size() - 1; i++) {
        for (int j = i + 1; j > 0; j--) {
            int swapIndex = j - 1;
            if (moveScores[swapIndex] < moveScores[j]) {
                qSwap(moves[j].moveValue, moves[swapIndex].moveValue);
                qSwap(moveScores[j], moveScores[swapIndex]);
            }
        }
    }
}
