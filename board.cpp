#include "board.h"
#include <QGraphicsSvgItem>
#include <QSvgRenderer>
#include "fennotation.h"
#include "zobrist.h"
Board::Board() {
    Zobrist = new class Zobrist;
}
Board::~Board(){
    delete Zobrist;
}
void Board::loadStartPosition(){
    loadPosition(FenNotation::startFen);
}

void Board::loadPosition(QString fen){
    Initialize();
    auto loadedPosition = FenNotation::PositionFromFen(fen);

    // Load pieces into board array and piece lists
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        int piece = loadedPosition.squares[squareIndex];
        Square[squareIndex] = piece;
        if (piece != Piece.None) {
            int pieceType = Piece.PieceType (piece);
            int pieceColourIndex = (Piece.IsColour (piece, Piece.White)) ? WhiteIndex : BlackIndex;
            if (Piece.IsSlidingPiece (piece)) {
                if (pieceType == Piece.Queen) {
                    queens[pieceColourIndex].AddPieceAtSquare (squareIndex);
                } else if (pieceType == Piece.Rook) {
                    rooks[pieceColourIndex].AddPieceAtSquare (squareIndex);
                } else if (pieceType == Piece.Bishop) {
                    bishops[pieceColourIndex].AddPieceAtSquare (squareIndex);
                }
            } else if (pieceType == Piece.Knight) {
                knights[pieceColourIndex].AddPieceAtSquare (squareIndex);
            } else if (pieceType == Piece.Pawn) {
                pawns[pieceColourIndex].AddPieceAtSquare (squareIndex);
            } else if (pieceType == Piece.King) {
                KingSquare[pieceColourIndex] = squareIndex;
            }
        }
    }

    // Side to move
    WhiteToMove = loadedPosition.whiteToMove;
    ColourToMove = (WhiteToMove) ? Piece.White : Piece.Black;
    OpponentColour = (WhiteToMove) ? Piece.Black : Piece.White;
    ColourToMoveIndex = (WhiteToMove) ? 0 : 1;

    // Create gamestate
    int whiteCastle = ((loadedPosition.whiteCastleKingside) ? 1 << 0 : 0) | ((loadedPosition.whiteCastleQueenside) ? 1 << 1 : 0);
    int blackCastle = ((loadedPosition.blackCastleKingside) ? 1 << 2 : 0) | ((loadedPosition.blackCastleQueenside) ? 1 << 3 : 0);
    int epState = loadedPosition.epFile << 4;
    ushort initialGameState = (ushort) (whiteCastle | blackCastle | epState);
    gameStateHistory.push (initialGameState);
    currentGameState = initialGameState;
    plyCount = loadedPosition.plyCount;

    // Initialize zobrist key
    ZobristKey = Zobrist->calculateZobristKey (this);
}

void Board::makeMove(Move &move, bool inSearch){
    uint32_t oldEnPassantFile = (currentGameState >> 4) & 15;
    uint32_t originalCastleState = currentGameState & 15;
    uint32_t newCastleState = originalCastleState;
    currentGameState = 0;

    int opponentColourIndex = 1 - ColourToMoveIndex;
    int moveFrom = move.StartSquare();
    int moveTo = move.TargetSquare();

    int capturedPieceType = Piece.PieceType (Square[moveTo]);
    int movePiece = Square[moveFrom];
    int movePieceType = Piece.PieceType (movePiece);

    int moveFlag = move.MoveFlag();
    bool isPromotion = move.IsPromotion();
    bool isEnPassant = moveFlag == Move::Flag::EnPassantCapture;

    // Handle captures
    currentGameState |= (short) (capturedPieceType << 8);
    if (capturedPieceType != 0 && !isEnPassant) {
        ZobristKey ^= Zobrist->piecesArray[capturedPieceType][opponentColourIndex][moveTo];
        GetPieceList(capturedPieceType, opponentColourIndex)->RemovePieceAtSquare (moveTo);
    }

    // Move pieces in piece lists
    if (movePieceType == Piece.King) {
        KingSquare[ColourToMoveIndex] = moveTo;
        newCastleState &= (WhiteToMove) ? whiteCastleMask : blackCastleMask;
    } else {
        GetPieceList(movePieceType, ColourToMoveIndex)->MovePiece (moveFrom, moveTo);
    }

    int pieceOnTargetSquare = movePiece;

    // Handle promotion
    if (isPromotion) {
        int promoteType = 0;
        if(moveFlag ==  Move::Flag::PromoteToQueen){
            promoteType = Piece.Queen;
            queens[ColourToMoveIndex].AddPieceAtSquare (moveTo);
        }
        if(moveFlag ==  Move::Flag::PromoteToRook){
            promoteType = Piece.Rook;
            rooks[ColourToMoveIndex].AddPieceAtSquare (moveTo);
        }
        if(moveFlag ==  Move::Flag::PromoteToBishop){
            promoteType = Piece.Bishop;
            bishops[ColourToMoveIndex].AddPieceAtSquare (moveTo);
        }
        if(moveFlag ==  Move::Flag::PromoteToKnight){
            promoteType = Piece.Knight;
            knights[ColourToMoveIndex].AddPieceAtSquare (moveTo);
        }
        pieceOnTargetSquare = promoteType | ColourToMove;
        pawns[ColourToMoveIndex].RemovePieceAtSquare (moveTo);
    } else {
        // Handle other special moves (en-passant, and castling)
        if(moveFlag == Move::Flag::EnPassantCapture){
            int epPawnSquare = moveTo + ((ColourToMove == Piece.White) ? -8 : 8);
            currentGameState |= (ushort) (Square[epPawnSquare] << 8); // add pawn as capture type
            Square[epPawnSquare] = 0; // clear ep capture square
            pawns[opponentColourIndex].RemovePieceAtSquare (epPawnSquare);
            ZobristKey ^= Zobrist->piecesArray[Piece.Pawn][opponentColourIndex][epPawnSquare];
        }
        if(moveFlag == Move::Flag::Castling){
            bool kingside = moveTo == BoardRepresentation::g1 || moveTo == BoardRepresentation::g8;
            int castlingRookFromIndex = (kingside) ? moveTo + 1 : moveTo - 2;
            int castlingRookToIndex = (kingside) ? moveTo - 1 : moveTo + 1;

            Square[castlingRookFromIndex] = Piece.None;
            Square[castlingRookToIndex] = Piece.Rook | ColourToMove;

            rooks[ColourToMoveIndex].MovePiece (castlingRookFromIndex, castlingRookToIndex);
            ZobristKey ^= Zobrist->piecesArray[Piece.Rook][ColourToMoveIndex][castlingRookFromIndex];
            ZobristKey ^= Zobrist->piecesArray[Piece.Rook][ColourToMoveIndex][castlingRookToIndex];
        }
    }

    // Update the board representation:
    Square[moveTo] = pieceOnTargetSquare;
    Square[moveFrom] = 0;

    // Pawn has moved two forwards, mark file with en-passant flag
    if (moveFlag == Move::Flag::PawnTwoForward) {
        int file = BoardRepresentation::FileIndex (moveFrom) + 1;
        currentGameState |= (ushort) (file << 4);
        ZobristKey ^= Zobrist->enPassantFile[file];
    }

    // Piece moving to/from rook square removes castling right for that side
    if (originalCastleState != 0) {
        if (moveTo == BoardRepresentation::h1 || moveFrom == BoardRepresentation::h1) {
            newCastleState &= whiteCastleKingsideMask;
        } else if (moveTo == BoardRepresentation::a1 || moveFrom == BoardRepresentation::a1) {
            newCastleState &= whiteCastleQueensideMask;
        }
        if (moveTo == BoardRepresentation::h8 || moveFrom == BoardRepresentation::h8) {
            newCastleState &= blackCastleKingsideMask;
        } else if (moveTo == BoardRepresentation::a8 || moveFrom == BoardRepresentation::a8) {
            newCastleState &= blackCastleQueensideMask;
        }
    }

    // Update zobrist key with new piece position and side to move
    ZobristKey ^= Zobrist->sideToMove;
    ZobristKey ^= Zobrist->piecesArray[movePieceType][ColourToMoveIndex][moveFrom];
    ZobristKey ^= Zobrist->piecesArray[Piece.PieceType (pieceOnTargetSquare)][ColourToMoveIndex][moveTo];

    if (oldEnPassantFile != 0)
        ZobristKey ^= Zobrist->enPassantFile[oldEnPassantFile];

    if (newCastleState != originalCastleState) {
        ZobristKey ^= Zobrist->castlingRights[originalCastleState]; // remove old castling rights state
        ZobristKey ^= Zobrist->castlingRights[newCastleState]; // add new castling rights state
    }
    currentGameState |= newCastleState;
    currentGameState |= (uint) fiftyMoveCounter << 14;
    gameStateHistory.push (currentGameState);

    // Change side to move
    WhiteToMove = !WhiteToMove;
    ColourToMove = (WhiteToMove) ? Piece.White : Piece.Black;
    OpponentColour = (WhiteToMove) ? Piece.Black : Piece.White;
    ColourToMoveIndex = 1 - ColourToMoveIndex;
    plyCount++;
    fiftyMoveCounter++;

    if (!inSearch) {
        if (movePieceType == Piece.Pawn || capturedPieceType != Piece.None) {
            RepetitionPositionHistory.clear ();
            fiftyMoveCounter = 0;
        } else {
            RepetitionPositionHistory.push (ZobristKey);
        }
    }
}

void Board::unmakeMove(Move &move, bool inSearch){
    //int opponentColour = ColourToMove;
    int opponentColourIndex = ColourToMoveIndex;
    bool undoingWhiteMove = OpponentColour == Piece.White;
    ColourToMove = OpponentColour; // side who made the move we are undoing
    OpponentColour = (undoingWhiteMove) ? Piece.Black : Piece.White;
    ColourToMoveIndex = 1 - ColourToMoveIndex;
    WhiteToMove = !WhiteToMove;

    uint32_t originalCastleState = currentGameState & 0b1111;

    int capturedPieceType = ((int) currentGameState >> 8) & 63;
    int capturedPiece = (capturedPieceType == 0) ? 0 : capturedPieceType | OpponentColour;

    int movedFrom = move.StartSquare();
    int movedTo = move.TargetSquare();
    int moveFlags = move.MoveFlag();
    bool isEnPassant = moveFlags == Move::Flag::EnPassantCapture;
    bool isPromotion = move.IsPromotion();

    int toSquarePieceType = Piece.PieceType (Square[movedTo]);
    int movedPieceType = (isPromotion) ? Piece.Pawn : toSquarePieceType;

    // Update zobrist key with new piece position and side to move
    ZobristKey ^= Zobrist->sideToMove;
    ZobristKey ^= Zobrist->piecesArray[movedPieceType][ColourToMoveIndex][movedFrom]; // add piece back to square it moved from
    ZobristKey ^= Zobrist->piecesArray[toSquarePieceType][ColourToMoveIndex][movedTo]; // remove piece from square it moved to

    uint32_t oldEnPassantFile = (currentGameState >> 4) & 15;
    if (oldEnPassantFile != 0)
        ZobristKey ^= Zobrist->enPassantFile[oldEnPassantFile];

    // ignore ep captures, handled later
    if (capturedPieceType != 0 && !isEnPassant) {
        ZobristKey ^= Zobrist->piecesArray[capturedPieceType][opponentColourIndex][movedTo];
        GetPieceList (capturedPieceType, opponentColourIndex)->AddPieceAtSquare (movedTo);
    }

    // Update king index
    if (movedPieceType == Piece.King) {
        KingSquare[ColourToMoveIndex] = movedFrom;
    } else if (!isPromotion) {
        GetPieceList (movedPieceType, ColourToMoveIndex)->MovePiece (movedTo, movedFrom);
    }

    // put back moved piece
    Square[movedFrom] = movedPieceType | ColourToMove; // note that if move was a pawn promotion, this will put the promoted piece back instead of the pawn. Handled in special move switch
    Square[movedTo] = capturedPiece; // will be 0 if no piece was captured

    if (isPromotion) {
        pawns[ColourToMoveIndex].AddPieceAtSquare (movedFrom);

        if(moveFlags == Move::Flag::PromoteToQueen){
            queens[ColourToMoveIndex].RemovePieceAtSquare (movedTo);
        }
        if(moveFlags == Move::Flag::PromoteToKnight){
            knights[ColourToMoveIndex].RemovePieceAtSquare (movedTo);
        }
        if(moveFlags == Move::Flag::PromoteToRook){
            rooks[ColourToMoveIndex].RemovePieceAtSquare (movedTo);
        }
        if(moveFlags == Move::Flag::PromoteToBishop){
            bishops[ColourToMoveIndex].RemovePieceAtSquare (movedTo);
        }
    } else if (isEnPassant) { // ep cature: put captured pawn back on right square
        int epIndex = movedTo + ((ColourToMove == Piece.White) ? -8 : 8);
        Square[movedTo] = 0;
        Square[epIndex] = (int) capturedPiece;
        pawns[opponentColourIndex].AddPieceAtSquare (epIndex);
        ZobristKey ^= Zobrist->piecesArray[Piece.Pawn][ opponentColourIndex][epIndex];
    } else if (moveFlags == Move::Flag::Castling) { // castles: move rook back to starting square

        bool kingside = movedTo == 6 || movedTo == 62;
        int castlingRookFromIndex = (kingside) ? movedTo + 1 : movedTo - 2;
        int castlingRookToIndex = (kingside) ? movedTo - 1 : movedTo + 1;

        Square[castlingRookToIndex] = 0;
        Square[castlingRookFromIndex] = Piece.Rook | ColourToMove;

        rooks[ColourToMoveIndex].MovePiece (castlingRookToIndex, castlingRookFromIndex);
        ZobristKey ^= Zobrist->piecesArray[Piece.Rook][ColourToMoveIndex][castlingRookFromIndex];
        ZobristKey ^= Zobrist->piecesArray[Piece.Rook][ColourToMoveIndex][castlingRookToIndex];
    }

    gameStateHistory.pop (); // removes current state from history
    currentGameState = gameStateHistory.top(); // sets current state to previous state in history

    fiftyMoveCounter = (int) (currentGameState & 4294950912) >> 14;
    int newEnPassantFile = (int) (currentGameState >> 4) & 15;
    if (newEnPassantFile != 0)
        ZobristKey ^= Zobrist->enPassantFile[newEnPassantFile];

    uint32_t newCastleState = currentGameState & 0b1111;
    if (newCastleState != originalCastleState) {
        ZobristKey ^= Zobrist->castlingRights[originalCastleState]; // remove old castling rights state
        ZobristKey ^= Zobrist->castlingRights[newCastleState]; // add new castling rights state
    }

    plyCount--;

    if (!inSearch && RepetitionPositionHistory.count() > 0) {
        RepetitionPositionHistory.pop();
    }

}


void Board::Initialize(){

    ZobristKey = 0;
    plyCount = 0;
    fiftyMoveCounter = 0;

    knights[0] = PieceList(10);
    pawns[0] = PieceList(8);
    rooks[0] = PieceList(10);
    bishops[0] = PieceList(10);
    queens[0] = PieceList(9);

    knights[1] = PieceList(10);
    pawns[1] = PieceList(8);
    rooks[1] = PieceList(10);
    bishops[1] = PieceList(10);
    queens[1] = PieceList(9);

    allPieceLists = {
        nullptr,
        nullptr,
        &pawns[WhiteIndex],
        &knights[WhiteIndex],
        nullptr,
        &bishops[WhiteIndex],
        &rooks[WhiteIndex],
        &queens[WhiteIndex],
        nullptr,
        nullptr,
        &pawns[BlackIndex],
        &knights[BlackIndex],
        nullptr,
        &bishops[BlackIndex],
        &rooks[BlackIndex],
        &queens[BlackIndex],
    };
}
