#include "evaluation.h"

Evaluation::Evaluation() {}

int Evaluation::Evaluate(Board booard){
    this->board = booard;
    int whiteEval = 0;
    int blackEval = 0;

    int whiteMaterial = CountMaterial (Board::WhiteIndex);
    int blackMaterial = CountMaterial (Board::BlackIndex);

    int whiteMaterialWithoutPawns = whiteMaterial - board.pawns[Board::WhiteIndex].Count() * pawnValue;
    int blackMaterialWithoutPawns = blackMaterial - board.pawns[Board::BlackIndex].Count() * pawnValue;
    float whiteEndgamePhaseWeight = EndgamePhaseWeight (whiteMaterialWithoutPawns);
    float blackEndgamePhaseWeight = EndgamePhaseWeight (blackMaterialWithoutPawns);

    whiteEval += whiteMaterial;
    blackEval += blackMaterial;
    whiteEval += MopUpEval (Board::WhiteIndex, Board::BlackIndex, whiteMaterial, blackMaterial, blackEndgamePhaseWeight);
    blackEval += MopUpEval (Board::BlackIndex, Board::WhiteIndex, blackMaterial, whiteMaterial, whiteEndgamePhaseWeight);

    whiteEval += EvaluatePieceSquareTables (Board::WhiteIndex, blackEndgamePhaseWeight);
    blackEval += EvaluatePieceSquareTables (Board::BlackIndex, whiteEndgamePhaseWeight);

    int eval = whiteEval - blackEval;

    int perspective = (board.WhiteToMove) ? 1 : -1;
    return eval * perspective;
}

int Evaluation::MopUpEval (int friendlyIndex, int opponentIndex, int myMaterial, int opponentMaterial, float endgameWeight) {
    int mopUpScore = 0;
    if (myMaterial > opponentMaterial + pawnValue * 2 && endgameWeight > 0) {

        int friendlyKingSquare = board.KingSquare[friendlyIndex];
        int opponentKingSquare = board.KingSquare[opponentIndex];
        mopUpScore += PMD.centreManhattanDistance[opponentKingSquare] * 10;
        // use ortho dst to promote direct opposition
        mopUpScore += (14 - PMD.NumRookMovesToReachSquare (friendlyKingSquare, opponentKingSquare)) * 4;

        return (int) (mopUpScore * endgameWeight);
    }
    return 0;
}

int Evaluation::CountMaterial (int colourIndex) {
    int material = 0;
    material += board.pawns[colourIndex].Count() * pawnValue;
    material += board.knights[colourIndex].Count() * knightValue;
    material += board.bishops[colourIndex].Count() * bishopValue;
    material += board.rooks[colourIndex].Count() * rookValue;
    material += board.queens[colourIndex].Count() * queenValue;

    return material;
}

int Evaluation::EvaluatePieceSquareTables (int colourIndex, float endgamePhaseWeight) {
    int value = 0;
    bool isWhite = colourIndex == Board::WhiteIndex;
    value += EvaluatePieceSquareTable (PieceSquareTable::pawns, board.pawns[colourIndex], isWhite);
    value += EvaluatePieceSquareTable (PieceSquareTable::rooks, board.rooks[colourIndex], isWhite);
    value += EvaluatePieceSquareTable (PieceSquareTable::knights, board.knights[colourIndex], isWhite);
    value += EvaluatePieceSquareTable (PieceSquareTable::bishops, board.bishops[colourIndex], isWhite);
    value += EvaluatePieceSquareTable (PieceSquareTable::queens, board.queens[colourIndex], isWhite);
    int kingEarlyPhase = PieceSquareTable::Read (PieceSquareTable::kingMiddle, board.KingSquare[colourIndex], isWhite);
    value += (int) (kingEarlyPhase * (1 - endgamePhaseWeight));
    //value += PieceSquareTable::Read (PieceSquareTable::kingMiddle, board.KingSquare[colourIndex], isWhite);

    return value;
}
