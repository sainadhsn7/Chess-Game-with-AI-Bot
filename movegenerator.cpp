#include "movegenerator.h"

MoveGenerator::MoveGenerator() {}
void MoveGenerator::printdebug(){
    qDebug() << "moves:" << moves.size();
    qDebug() << "inCheck:" << inCheck;
    qDebug() << "inDoubleCheck:" << inDoubleCheck;
    qDebug() << "pinsExistInPosition:" << pinsExistInPosition;
    qDebug() << "checkRayBitmask:" << checkRayBitmask;
    qDebug() << "pinRayBitmask:" << pinRayBitmask;
    // qDebug() << "isWhiteToMove:" << isWhiteToMove;

    // qDebug() << "friendlyColour:" << friendlyColour;
    // qDebug() << "opponentColour:" << opponentColour;
    // qDebug() << "friendlyKingSquare:" << friendlyKingSquare;
    // qDebug() << "friendlyColourIndex:" << friendlyColourIndex;
    // qDebug() << "opponentColourIndex:" << opponentColourIndex;
    qDebug() << "pawnattack bitboard::" << PMD.pawnAttackBitboards[0][0];
    qDebug() << "\n";
}
QList<Move> MoveGenerator:: GenerateMoves (Board *board, bool includeQuietMoves) {
    this->board = board;
    genQuiets = includeQuietMoves;
    Init ();
    CalculateAttackData ();
    GenerateKingMoves ();
    // Only king moves are valid in a double check position, so can return early.
    if (inDoubleCheck) {
        return moves;
    }
    GenerateSlidingMoves ();
    GenerateKnightMoves ();
    GeneratePawnMoves ();
    printdebug();
    return moves;
}
void MoveGenerator::Init(){
    moves.clear();
    moves.reserve(100);
    inCheck = false;
    inDoubleCheck = false;
    pinsExistInPosition = false;
    checkRayBitmask = 0;
    pinRayBitmask = 0;

    isWhiteToMove = board->ColourToMove == Piece::White;
    friendlyColour = board->ColourToMove;
    opponentColour = board->OpponentColour;
    friendlyKingSquare = board->KingSquare[board->ColourToMoveIndex];
    friendlyColourIndex = (board->WhiteToMove) ? Board::WhiteIndex : Board::BlackIndex;
    opponentColourIndex = 1 - friendlyColourIndex;
}
void MoveGenerator::GenerateKingMoves(){
    for (int i = 0; i < PMD.kingMoves[friendlyKingSquare].size(); i++) {
        int targetSquare = PMD.kingMoves[friendlyKingSquare][i];
        int pieceOnTargetSquare = board->Square[targetSquare];

        // Skip squares occupied by friendly pieces
        if (Piece::IsColour (pieceOnTargetSquare, friendlyColour)) {
            continue;
        }

        bool isCapture = Piece::IsColour (pieceOnTargetSquare, opponentColour);
        if (!isCapture) {
            // King can't move to square marked as under enemy control, unless he is capturing that piece
            // Also skip if not generating quiet moves
            if (!genQuiets || SquareIsInCheckRay (targetSquare)) {
                continue;
            }
        }

        // Safe for king to move to this square
        if (!SquareIsAttacked (targetSquare)) {
            moves.push_back(Move (friendlyKingSquare, targetSquare));

            // Castling:
            if (!inCheck && !isCapture) {
                // Castle kingside
                if ((targetSquare == BR::f1 || targetSquare == BR::f8) && HasKingsideCastleRight()) {
                    int castleKingsideSquare = targetSquare + 1;
                    if (board->Square[castleKingsideSquare] == Piece::None) {
                        if (!SquareIsAttacked (castleKingsideSquare)) {
                            moves.push_back (Move(friendlyKingSquare, castleKingsideSquare, Move::Flag::Castling));
                        }
                    }
                }
                // Castle queenside
                else if ((targetSquare == BR::d1 || targetSquare == BR::d8) && HasQueensideCastleRight()) {
                    int castleQueensideSquare = targetSquare - 1;
                    if (board->Square[castleQueensideSquare] == Piece::None && board->Square[castleQueensideSquare - 1] == Piece::None) {
                        if (!SquareIsAttacked (castleQueensideSquare)) {
                            moves.push_back(Move (friendlyKingSquare, castleQueensideSquare, Move::Flag::Castling));
                        }
                    }
                }
            }
        }
    }
}

void MoveGenerator::GenerateSlidingMoves () {
    PieceList rooks = board->rooks[friendlyColourIndex];
    for (int i = 0; i < rooks.Count(); i++) {
        GenerateSlidingPieceMoves (rooks[i], 0, 4);
    }

    PieceList bishops = board->bishops[friendlyColourIndex];
    for (int i = 0; i < bishops.Count(); i++) {
        GenerateSlidingPieceMoves (bishops[i], 4, 8);
    }

    PieceList queens = board->queens[friendlyColourIndex];
    for (int i = 0; i < queens.Count(); i++) {
        GenerateSlidingPieceMoves (queens[i], 0, 8);
    }

}

void MoveGenerator::GenerateSlidingPieceMoves (int startSquare, int startDirIndex, int endDirIndex) {
    bool isPinned = IsPinned (startSquare);

    // If this piece is pinned, and the king is in check, this piece cannot move
    if (inCheck && isPinned) {
        return;
    }

    for (int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++) {
        int currentDirOffset = PMD.directionOffsets[directionIndex];

        // If pinned, this piece can only move along the ray towards/away from the friendly king, so skip other directions
        if (isPinned && !IsMovingAlongRay (currentDirOffset, friendlyKingSquare, startSquare)) {
            continue;
        }

        for (int n = 0; n < PMD.numSquaresToEdge[startSquare][directionIndex]; n++) {
            int targetSquare = startSquare + currentDirOffset * (n + 1);
            int targetSquarePiece = board->Square[targetSquare];

            // Blocked by friendly piece, so stop looking in this direction
            if (Piece::IsColour (targetSquarePiece, friendlyColour)) {
                break;
            }
            bool isCapture = targetSquarePiece != Piece::None;

            bool movePreventsCheck = SquareIsInCheckRay (targetSquare);
            if (movePreventsCheck || !inCheck) {
                if (genQuiets || isCapture) {
                    moves.push_back(Move (startSquare, targetSquare));
                }
            }
            // If square not empty, can't move any further in this direction
            // Also, if this move blocked a check, further moves won't block the check
            if (isCapture || movePreventsCheck) {
                break;
            }
        }
    }
}

void MoveGenerator::GenerateKnightMoves () {
    PieceList myKnights = board->knights[friendlyColourIndex];

    for (int i = 0; i < myKnights.Count(); i++) {
        int startSquare = myKnights[i];

        // Knight cannot move if it is pinned
        if (IsPinned (startSquare)) {
            continue;
        }

        for (int knightMoveIndex = 0; knightMoveIndex < PMD.knightMoves[startSquare].size(); knightMoveIndex++) {
            int targetSquare = PMD.knightMoves[startSquare][knightMoveIndex];
            int targetSquarePiece = board->Square[targetSquare];
            bool isCapture = Piece::IsColour (targetSquarePiece, opponentColour);
            if (genQuiets || isCapture) {
                // Skip if square contains friendly piece, or if in check and knight is not interposing/capturing checking piece
                if (Piece::IsColour (targetSquarePiece, friendlyColour) || (inCheck && !SquareIsInCheckRay (targetSquare))) {
                    continue;
                }
                moves.push_back(Move (startSquare, targetSquare));
            }
        }
    }
}

void MoveGenerator::GeneratePawnMoves () {
    PieceList myPawns = board->pawns[friendlyColourIndex];
    int pawnOffset = (friendlyColour == Piece::White) ? 8 : -8;
    int startRank = (board->WhiteToMove) ? 1 : 6;
    int finalRankBeforePromotion = (board->WhiteToMove) ? 6 : 1;

    int enPassantFile = ((int) (board->currentGameState >> 4) & 15) - 1;
    int enPassantSquare = -1;
    if (enPassantFile != -1) {
        enPassantSquare = 8 * ((board->WhiteToMove) ? 5 : 2) + enPassantFile;
    }

    for (int i = 0; i < myPawns.Count(); i++) {
        int startSquare = myPawns[i];
        int rank = BR::RankIndex (startSquare);
        bool oneStepFromPromotion = rank == finalRankBeforePromotion;

        if (genQuiets) {

            int squareOneForward = startSquare + pawnOffset;

            // Square ahead of pawn is empty: forward moves
            if (board->Square[squareOneForward] == Piece::None) {
                // Pawn not pinned, or is moving along line of pin
                if (!IsPinned (startSquare) || IsMovingAlongRay (pawnOffset, startSquare, friendlyKingSquare)) {
                    // Not in check, or pawn is interposing checking piece
                    if (!inCheck || SquareIsInCheckRay (squareOneForward)) {
                        if (oneStepFromPromotion) {
                            MakePromotionMoves (startSquare, squareOneForward);
                        } else {
                            moves.push_back (Move (startSquare, squareOneForward));
                        }
                    }

                    // Is on starting square (so can move two forward if not blocked)
                    if (rank == startRank) {
                        int squareTwoForward = squareOneForward + pawnOffset;
                        if (board->Square[squareTwoForward] == Piece::None) {
                            // Not in check, or pawn is interposing checking piece
                            if (!inCheck || SquareIsInCheckRay (squareTwoForward)) {
                                moves.push_back (Move (startSquare, squareTwoForward, Move::Flag::PawnTwoForward));
                            }
                        }
                    }
                }
            }
        }

        // Pawn captures.
        for (int j = 0; j < 2; j++) {
            // Check if square exists diagonal to pawn
            if (PMD.numSquaresToEdge[startSquare][PMD.pawnAttackDirections[friendlyColourIndex][j]] > 0) {
                // move in direction friendly pawns attack to get square from which enemy pawn would attack
                int pawnCaptureDir = PMD.directionOffsets[PMD.pawnAttackDirections[friendlyColourIndex][j]];
                int targetSquare = startSquare + pawnCaptureDir;
                int targetPiece = board->Square[targetSquare];

                // If piece is pinned, and the square it wants to move to is not on same line as the pin, then skip this direction
                if (IsPinned (startSquare) && !IsMovingAlongRay (pawnCaptureDir, friendlyKingSquare, startSquare)) {
                    continue;
                }

                // Regular capture
                if (Piece::IsColour (targetPiece, opponentColour)) {
                    // If in check, and piece is not capturing/interposing the checking piece, then skip to next square
                    if (inCheck && !SquareIsInCheckRay (targetSquare)) {
                        continue;
                    }
                    if (oneStepFromPromotion) {
                        MakePromotionMoves (startSquare, targetSquare);
                    } else {
                        moves.push_back (Move (startSquare, targetSquare));
                    }
                }

                // Capture en-passant
                if (targetSquare == enPassantSquare) {
                    int epCapturedPawnSquare = targetSquare + ((board->WhiteToMove) ? -8 : 8);
                    if (!InCheckAfterEnPassant (startSquare, targetSquare, epCapturedPawnSquare)) {
                        moves.push_back(Move (startSquare, targetSquare, Move::Flag::EnPassantCapture));
                    }
                }
            }
        }
    }
}

void MoveGenerator::MakePromotionMoves (int fromSquare, int toSquare) {
    moves.push_back (Move (fromSquare, toSquare, Move::Flag::PromoteToQueen));
    if (promotionsToGenerate == PromotionMode::All) {
        moves.push_back (Move (fromSquare, toSquare, Move::Flag::PromoteToKnight));
        moves.push_back (Move (fromSquare, toSquare, Move::Flag::PromoteToRook));
        moves.push_back (Move (fromSquare, toSquare, Move::Flag::PromoteToBishop));
    } else if (promotionsToGenerate == PromotionMode::QueenAndKnight) {
        moves.push_back (Move (fromSquare, toSquare, Move::Flag::PromoteToKnight));
    }
}

void MoveGenerator::GenSlidingAttackMap () {
    opponentSlidingAttackMap = 0;

    PieceList enemyRooks = board->rooks[opponentColourIndex];
    for (int i = 0; i < enemyRooks.Count(); i++) {
        UpdateSlidingAttackPiece (enemyRooks[i], 0, 4);
    }

    PieceList enemyQueens = board->queens[opponentColourIndex];
    for (int i = 0; i < enemyQueens.Count(); i++) {
        UpdateSlidingAttackPiece (enemyQueens[i], 0, 8);
    }

    PieceList enemyBishops = board->bishops[opponentColourIndex];
    for (int i = 0; i < enemyBishops.Count(); i++) {
        UpdateSlidingAttackPiece (enemyBishops[i], 4, 8);
    }
}

void MoveGenerator::UpdateSlidingAttackPiece (int startSquare, int startDirIndex, int endDirIndex) {

    for (int directionIndex = startDirIndex; directionIndex < endDirIndex; directionIndex++) {
        int currentDirOffset = PMD.directionOffsets[directionIndex];
        for (int n = 0; n < PMD.numSquaresToEdge[startSquare][directionIndex]; n++) {
            int targetSquare = startSquare + currentDirOffset * (n + 1);
            int targetSquarePiece = board->Square[targetSquare];
            opponentSlidingAttackMap |= 1ull << targetSquare;
            if (targetSquare != friendlyKingSquare) {
                if (targetSquarePiece != Piece::None) {
                    break;
                }
            }
        }
    }
}

void MoveGenerator::CalculateAttackData () {
    GenSlidingAttackMap ();
    // Search squares in all directions around friendly king for checks/pins by enemy sliding pieces (queen, rook, bishop)
    int startDirIndex = 0;
    int endDirIndex = 8;

    if (board->queens[opponentColourIndex].Count() == 0) {
        startDirIndex = (board->rooks[opponentColourIndex].Count() > 0) ? 0 : 4;
        endDirIndex = (board->bishops[opponentColourIndex].Count() > 0) ? 8 : 4;
    }

    for (int dir = startDirIndex; dir < endDirIndex; dir++) {
        bool isDiagonal = dir > 3;

        int n = PMD.numSquaresToEdge[friendlyKingSquare][dir];
        int directionOffset = PMD.directionOffsets[dir];
        bool isFriendlyPieceAlongRay = false;
        uint64_t rayMask = 0;

        for (int i = 0; i < n; i++) {
            int squareIndex = friendlyKingSquare + directionOffset * (i + 1);
            rayMask |= 1ull << squareIndex;
            int piece = board->Square[squareIndex];

            // This square contains a piece
            if (piece != Piece::None) {
                if (Piece::IsColour (piece, friendlyColour)) {
                    // First friendly piece we have come across in this direction, so it might be pinned
                    if (!isFriendlyPieceAlongRay) {
                        isFriendlyPieceAlongRay = true;
                    }
                    // This is the second friendly piece we've found in this direction, therefore pin is not possible
                    else {
                        break;
                    }
                }
                // This square contains an enemy piece
                else {
                    int pieceType = Piece::PieceType (piece);

                    // Check if piece is in bitmask of pieces able to move in current direction
                    if ((isDiagonal && Piece::IsBishopOrQueen (pieceType)) || (!isDiagonal && Piece::IsRookOrQueen (pieceType))) {
                        // Friendly piece blocks the check, so this is a pin
                        if (isFriendlyPieceAlongRay) {
                            pinsExistInPosition = true;
                            pinRayBitmask |= rayMask;
                        }
                        // No friendly piece blocking the attack, so this is a check
                        else {
                            checkRayBitmask |= rayMask;
                            inDoubleCheck = inCheck; // if already in check, then this is double check
                            inCheck = true;
                        }
                        break;
                    } else {
                        // This enemy piece is not able to move in the current direction, and so is blocking any checks/pins
                        break;
                    }
                }
            }
        }
        // Stop searching for pins if in double check, as the king is the only piece able to move in that case anyway
        if (inDoubleCheck) {
            break;
        }

    }
    // Knight attacks
    PieceList opponentKnights = board->knights[opponentColourIndex];
    opponentKnightAttacks = 0;
    bool isKnightCheck = false;
    for (int knightIndex = 0; knightIndex < opponentKnights.Count(); knightIndex++) {
        int startSquare = opponentKnights[knightIndex];
        opponentKnightAttacks |= PMD.knightAttackBitboards[startSquare];
        if (!isKnightCheck && BitBoard::ContainsSquare (opponentKnightAttacks, friendlyKingSquare)) {
            isKnightCheck = true;
            inDoubleCheck = inCheck; // if already in check, then this is double check
            inCheck = true;
            checkRayBitmask |= 1ull << startSquare;
        }
    }
    // Pawn attacks
    PieceList opponentPawns = board->pawns[opponentColourIndex];
    opponentPawnAttackMap = 0;
    bool isPawnCheck = false;

    for (int pawnIndex = 0; pawnIndex < opponentPawns.Count(); pawnIndex++) {
        int pawnSquare = opponentPawns[pawnIndex];
        uint64_t pawnAttacks = PMD.pawnAttackBitboards[pawnSquare][opponentColourIndex];
        opponentPawnAttackMap |= pawnAttacks;

        if (!isPawnCheck && BitBoard::ContainsSquare (pawnAttacks, friendlyKingSquare)) {
            isPawnCheck = true;
            inDoubleCheck = inCheck; // if already in check, then this is double check
            inCheck = true;
            checkRayBitmask |= 1ull << pawnSquare;
        }
    }

    int enemyKingSquare = board->KingSquare[opponentColourIndex];

    opponentAttackMapNoPawns = opponentSlidingAttackMap | opponentKnightAttacks | PMD.kingAttackBitboards[enemyKingSquare];
    opponentAttackMap = opponentAttackMapNoPawns | opponentPawnAttackMap;
}

bool MoveGenerator::InCheckAfterEnPassant (int startSquare, int targetSquare, int epCapturedPawnSquare) {
    // Update board to reflect en-passant capture
    board->Square[targetSquare] = board->Square[startSquare];
    board->Square[startSquare] = Piece::None;
    board->Square[epCapturedPawnSquare] = Piece::None;

    bool inCheckAfterEpCapture = false;
    if (SquareAttackedAfterEPCapture (epCapturedPawnSquare, startSquare)) {
        inCheckAfterEpCapture = true;
    }

    // Undo change to board
    board->Square[targetSquare] = Piece::None;
    board->Square[startSquare] = Piece::Pawn | friendlyColour;
    board->Square[epCapturedPawnSquare] = Piece::Pawn | opponentColour;
    return inCheckAfterEpCapture;
}

bool MoveGenerator::SquareAttackedAfterEPCapture (int epCaptureSquare, int capturingPawnStartSquare) {
    if (BitBoard::ContainsSquare (opponentAttackMapNoPawns, friendlyKingSquare)) {
        return true;
    }

    // Loop through the horizontal direction towards ep capture to see if any enemy piece now attacks king
    int dirIndex = (epCaptureSquare < friendlyKingSquare) ? 2 : 3;
    for (int i = 0; i < PMD.numSquaresToEdge[friendlyKingSquare][dirIndex]; i++) {
        int squareIndex = friendlyKingSquare + PMD.directionOffsets[dirIndex] * (i + 1);
        int piece = board->Square[squareIndex];
        if (piece != Piece::None) {
            // Friendly piece is blocking view of this square from the enemy.
            if (Piece::IsColour (piece, friendlyColour)) {
                break;
            }
            // This square contains an enemy piece
            else {
                if (Piece::IsRookOrQueen (piece)) {
                    return true;
                } else {
                    // This piece is not able to move in the current direction, and is therefore blocking any checks along this line
                    break;
                }
            }
        }
    }

    // check if enemy pawn is controlling this square (can't use pawn attack bitboard, because pawn has been captured)
    for (int i = 0; i < 2; i++) {
        // Check if square exists diagonal to friendly king from which enemy pawn could be attacking it
        if (PMD.numSquaresToEdge[friendlyKingSquare][PMD.pawnAttackDirections[friendlyColourIndex][i]] > 0) {
            // move in direction friendly pawns attack to get square from which enemy pawn would attack
            int piece = board->Square[friendlyKingSquare + PMD.directionOffsets[PMD.pawnAttackDirections[friendlyColourIndex][i]]];
            if (piece == (Piece::Pawn | opponentColour)) // is enemy pawn
            {
                return true;
            }
        }
    }

    return false;
}


////////////////////////////////////////  OTHER OPERATORS AND CONSTRUCTORS   /////////////////////////////////////////////////
MoveGenerator::MoveGenerator(const MoveGenerator &other)
    : promotionsToGenerate(other.promotionsToGenerate),
    opponentAttackMap(other.opponentAttackMap),
    opponentPawnAttackMap(other.opponentPawnAttackMap),
    PMD(other.PMD),
    moves(other.moves),
    isWhiteToMove(other.isWhiteToMove),
    friendlyColour(other.friendlyColour),
    opponentColour(other.opponentColour),
    friendlyKingSquare(other.friendlyKingSquare),
    friendlyColourIndex(other.friendlyColourIndex),
    opponentColourIndex(other.opponentColourIndex),
    inCheck(other.inCheck),
    inDoubleCheck(other.inDoubleCheck),
    pinsExistInPosition(other.pinsExistInPosition),
    checkRayBitmask(other.checkRayBitmask),
    pinRayBitmask(other.pinRayBitmask),
    opponentKnightAttacks(other.opponentKnightAttacks),
    opponentAttackMapNoPawns(other.opponentAttackMapNoPawns),
    opponentSlidingAttackMap(other.opponentSlidingAttackMap),
    genQuiets(other.genQuiets),
    board(other.board) {
    // Perform deep copy if necessary
}

MoveGenerator::MoveGenerator(MoveGenerator &&other) noexcept
    : promotionsToGenerate(std::move(other.promotionsToGenerate)),
    opponentAttackMap(std::move(other.opponentAttackMap)),
    opponentPawnAttackMap(std::move(other.opponentPawnAttackMap)),
    PMD(std::move(other.PMD)),
    moves(std::move(other.moves)),
    isWhiteToMove(other.isWhiteToMove),
    friendlyColour(other.friendlyColour),
    opponentColour(other.opponentColour),
    friendlyKingSquare(other.friendlyKingSquare),
    friendlyColourIndex(other.friendlyColourIndex),
    opponentColourIndex(other.opponentColourIndex),
    inCheck(other.inCheck),
    inDoubleCheck(other.inDoubleCheck),
    pinsExistInPosition(other.pinsExistInPosition),
    checkRayBitmask(other.checkRayBitmask),
    pinRayBitmask(other.pinRayBitmask),
    opponentKnightAttacks(other.opponentKnightAttacks),
    opponentAttackMapNoPawns(other.opponentAttackMapNoPawns),
    opponentSlidingAttackMap(other.opponentSlidingAttackMap),
    genQuiets(other.genQuiets),
    board(other.board) {
    // Nullify other's pointer to avoid double deletion
    other.board = nullptr;
}

MoveGenerator& MoveGenerator::operator=(const MoveGenerator &other) {
    if (this == &other)
        return *this;

    // Copy data from other
    promotionsToGenerate = other.promotionsToGenerate;
    opponentAttackMap = other.opponentAttackMap;
    opponentPawnAttackMap = other.opponentPawnAttackMap;
    moves = other.moves;
    isWhiteToMove = other.isWhiteToMove;
    friendlyColour = other.friendlyColour;
    opponentColour = other.opponentColour;
    friendlyKingSquare = other.friendlyKingSquare;
    friendlyColourIndex = other.friendlyColourIndex;
    opponentColourIndex = other.opponentColourIndex;
    inCheck = other.inCheck;
    inDoubleCheck = other.inDoubleCheck;
    pinsExistInPosition = other.pinsExistInPosition;
    checkRayBitmask = other.checkRayBitmask;
    pinRayBitmask = other.pinRayBitmask;
    opponentKnightAttacks = other.opponentKnightAttacks;
    opponentAttackMapNoPawns = other.opponentAttackMapNoPawns;
    opponentSlidingAttackMap = other.opponentSlidingAttackMap;
    genQuiets = other.genQuiets;
    board = other.board;

    return *this;
}

MoveGenerator& MoveGenerator::operator=(MoveGenerator &&other) noexcept {
    if (this == &other)
        return *this;

    // Move data from other
    promotionsToGenerate = std::move(other.promotionsToGenerate);
    opponentAttackMap = std::move(other.opponentAttackMap);
    opponentPawnAttackMap = std::move(other.opponentPawnAttackMap);
    moves = std::move(other.moves);
    isWhiteToMove = other.isWhiteToMove;
    friendlyColour = other.friendlyColour;
    opponentColour = other.opponentColour;
    friendlyKingSquare = other.friendlyKingSquare;
    friendlyColourIndex = other.friendlyColourIndex;
    opponentColourIndex = other.opponentColourIndex;
    inCheck = other.inCheck;
    inDoubleCheck = other.inDoubleCheck;
    pinsExistInPosition = other.pinsExistInPosition;
    checkRayBitmask = other.checkRayBitmask;
    pinRayBitmask = other.pinRayBitmask;
    opponentKnightAttacks = other.opponentKnightAttacks;
    opponentAttackMapNoPawns = other.opponentAttackMapNoPawns;
    opponentSlidingAttackMap = other.opponentSlidingAttackMap;
    genQuiets = other.genQuiets;
    board = other.board;

    // Nullify other's pointer to avoid double deletion
    other.board = nullptr;

    return *this;
}

