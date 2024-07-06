#include "fennotation.h"
#include "BoardRepresentation.h"
#include "board.h"
#include <QDebug>

const QString FenNotation:: startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const QMap<QChar, int> FenNotation:: pieceTypeFromSymbol = {
{'k', Piece::King},
{'p', Piece::Pawn},
{'n', Piece::Knight},
{'b', Piece::Bishop},
{'r', Piece::Rook},
{'q', Piece::Queen}
};
FenNotation::FenNotation() {}
FenNotation::LoadedPositionInfo FenNotation::PositionFromFen(QString fen){

    LoadedPositionInfo loadedPositionInfo;
    int col = 0, row = 7;
    auto section = fen.split(' ');

    for(int i=0; i < section[0].length(); i++){
        auto symbol = section[0][i];
        if(symbol == ' ')break;
        if(symbol == '/'){
            col = 0, row--;
        }
        else{
            if(symbol.isDigit()){
                col += symbol.digitValue();
            }
            else{
                int pieceColour = symbol.isUpper() ? Piece::White:Piece::Black;
                int pieceType = pieceTypeFromSymbol[symbol.toLower()];
                loadedPositionInfo.squares[row*8 + col] = pieceColour | pieceType;
                col++;
            }
        }
    }
    loadedPositionInfo.whiteToMove = section[1] == "w";

    QString castlingRights = section.length()>2 ? section[2]: "KQkq";
    loadedPositionInfo.whiteCastleKingside = castlingRights.contains('K');
    loadedPositionInfo.whiteCastleQueenside = castlingRights.contains('Q');
    loadedPositionInfo.blackCastleKingside = castlingRights.contains('k');
    loadedPositionInfo.blackCastleQueenside = castlingRights.contains('q');

    BoardRepresentation boardRepresentation;
    if (section.length() > 3) {
        QString enPassantFileName = QString(section[3][0]);
        if (boardRepresentation.fileNames.contains (enPassantFileName)) {
            loadedPositionInfo.epFile = boardRepresentation.fileNames.indexOf(enPassantFileName) + 1;
        }
    }
    if (section.length() > 4){
        try{
            loadedPositionInfo.plyCount = section[4].toInt();
        }catch (const std::invalid_argument& e) {
            loadedPositionInfo.plyCount = 0;
        }catch (const std::out_of_range& e) {
            loadedPositionInfo.plyCount = 0;
        }
    }
    return loadedPositionInfo;
}

QString FenNotation::CurrentFen(Board* board){
    QString fen = "";
    for (int rank = 7; rank >= 0; rank--) {
        int numEmptyFiles = 0;
        for (int file = 0; file < 8; file++) {

            int i = rank * 8 + file;
            int piece = board->Square[i];
            if (piece != Piece::None) {
                if (numEmptyFiles != 0) {
                    fen += QString::number(numEmptyFiles);
                    numEmptyFiles = 0;
                }
                bool isBlack = Piece::IsColour (piece, Piece::Black);
                int pieceType = Piece::PieceType (piece);
                QChar pieceChar = ' ';
                if(pieceType == Piece::Rook)
                    pieceChar = 'R';
                if(pieceType == Piece::Knight)
                    pieceChar = 'N';
                if(pieceType == Piece::Bishop)
                    pieceChar = 'B';
                if(pieceType == Piece::Queen)
                    pieceChar = 'Q';
                if(pieceType == Piece::King)
                    pieceChar = 'K';
                if(pieceType == Piece::Pawn)
                    pieceChar = 'P';
                fen += (isBlack) ? pieceChar.toLower() : pieceChar;
            }
            else {
                numEmptyFiles++;
            }

        }
        if (numEmptyFiles != 0) {
            fen += QString::number(numEmptyFiles);
        }
        if (rank != 0) {
            fen += '/';
        }
    }
    fen += ' ';
    fen += (board->WhiteToMove) ? 'w' : 'b';

    // Castling
    bool whiteKingside = (board->currentGameState & 1) == 1;
    bool whiteQueenside = (board->currentGameState >> 1 & 1) == 1;
    bool blackKingside = (board->currentGameState >> 2 & 1) == 1;
    bool blackQueenside = (board->currentGameState >> 3 & 1) == 1;
    fen += ' ';
    fen += (whiteKingside) ? "K" : "";
    fen += (whiteQueenside) ? "Q" : "";
    fen += (blackKingside) ? "k" : "";
    fen += (blackQueenside) ? "q" : "";
    fen += ((board->currentGameState & 15) == 0) ? "-" : "";

    // En-passant
    fen += ' ';
    int epFile = (int) (board->currentGameState >> 4) & 15;
    if (epFile == 0) {
        fen += '-';
    } else {
        QString fileName = (new BoardRepresentation)->fileNames[epFile - 1];
        int epRank = (board->WhiteToMove) ? 6 : 3;
        fen += fileName + QString::number(epRank);
    }

    // 50 move counter
    fen += ' ';
    fen += QString::number(board->fiftyMoveCounter);

    // Full-move count (should be one at start, and increase after each move by black)
    fen += ' ';
    fen += QString::number((board->plyCount / 2) + 1);
    return fen;
}
