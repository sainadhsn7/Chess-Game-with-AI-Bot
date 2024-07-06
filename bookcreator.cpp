#include "bookcreator.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "movegenerator.h"

BookCreator::BookCreator() {}
Book BookCreator::loadBookFromFile(const QString &bookFilePath) {
    Book book;
    QFile file(bookFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            if (!line.isEmpty()) {
                uint64_t positionKey = line.split(':')[0].toULongLong();
                QStringList moveInfoStrings = line.split(':')[1].trimmed().split(',');

                for (const QString &moveInfoString : moveInfoStrings) {
                    QStringList moveInfo = moveInfoString.trimmed().split(' ');
                    if (moveInfo.size() == 2) {
                        ushort moveValue = moveInfo[0].toUShort();
                        int numTimesPlayed = moveInfo[1].remove("(").remove(")").toInt();
                        book.Add(positionKey, Move(moveValue), numTimesPlayed);
                    }
                }
            }
        }
        file.close();
    } else {
        qWarning() << "Could not open book file" << bookFilePath;
    }
    return book;
}


QString PGNCreator::CreatePGN (QList<Move> moves) {
    QString pgn = "";
    Board board;
    board.loadStartPosition ();

    for (int plyCount = 0; plyCount < moves.length(); plyCount++) {
        QString moveString = NotationFromMove (board, moves[plyCount]);
        board.makeMove (moves[plyCount]);

        if (plyCount % 2 == 0) {
            pgn += QString::number((plyCount / 2) + 1) + ". ";
        }
        pgn += moveString + " ";
    }

    return pgn;
}

QString PGNCreator::NotationFromMove (QString currentFen, Move &move) {
    Board board;
    board.loadPosition (currentFen);
    return NotationFromMove (board, move);
}

QString PGNCreator::NotationFromMove (Board &board, Move &move) {

    MoveGenerator moveGen;

    int movePieceType = Piece::PieceType (board.Square[move.StartSquare()]);
    int capturedPieceType = Piece::PieceType (board.Square[move.TargetSquare()]);

    if (move.MoveFlag() == Move::Flag::Castling) {
        int delta = move.TargetSquare() - move.StartSquare();
        if (delta == 2) {
            return "O-O";
        } else if (delta == -2) {
            return "O-O-O";
        }
    }

    QString moveNotation = GetSymbolFromPieceType (movePieceType);

    // check if any ambiguity exists in notation (e.g if e2 can be reached via Nfe2 and Nbe2)
    if (movePieceType != Piece::Pawn && movePieceType != Piece::King) {
        auto allMoves = moveGen.GenerateMoves (&board);

        for (auto &altMove: allMoves) {

            if (altMove.StartSquare() != move.StartSquare() && altMove.TargetSquare() == move.TargetSquare()) { // if moving to same square from different square
                if (Piece::PieceType (board.Square[altMove.StartSquare()]) == movePieceType) { // same piece type
                    int fromFileIndex = BoardRepresentation::FileIndex (move.StartSquare());
                    int alternateFromFileIndex = BoardRepresentation::FileIndex (altMove.StartSquare());
                    int fromRankIndex = BoardRepresentation::RankIndex (move.StartSquare());
                    int alternateFromRankIndex = BoardRepresentation::RankIndex (altMove.StartSquare());

                    if (fromFileIndex != alternateFromFileIndex) { // pieces on different files, thus ambiguity can be resolved by specifying file
                        moveNotation += BoardRepresentation::fileNames[fromFileIndex];
                        break; // ambiguity resolved
                    } else if (fromRankIndex != alternateFromRankIndex) {
                        moveNotation += BoardRepresentation::rankNames[fromRankIndex];
                        break; // ambiguity resolved
                    }
                }
            }

        }
    }

    if (capturedPieceType != 0) { // add 'x' to indicate capture
        if (movePieceType == Piece::Pawn) {
            moveNotation += BoardRepresentation::fileNames[BoardRepresentation::FileIndex (move.StartSquare())];
        }
        moveNotation += "x";
    } else { // check if capturing ep
        if (move.MoveFlag() == Move::Flag::EnPassantCapture) {
            moveNotation += QString(BoardRepresentation::fileNames[BoardRepresentation::FileIndex (move.StartSquare())]) + "x";
        }
    }

    moveNotation += BoardRepresentation::fileNames[BoardRepresentation::FileIndex (move.TargetSquare())];
    moveNotation += BoardRepresentation::rankNames[BoardRepresentation::RankIndex (move.TargetSquare())];

    // add promotion piece
    if (move.IsPromotion()) {
        int promotionPieceType = move.PromotionPieceType();
        moveNotation += "=" + GetSymbolFromPieceType (promotionPieceType);
    }

    board.makeMove (move, true);
    auto legalResponses = moveGen.GenerateMoves (&board);
    // add check/mate symbol if applicable
    if (moveGen.InCheck ()) {
        if (legalResponses.count() == 0) {
            moveNotation += "#";
        } else {
            moveNotation += "+";
        }
    }
    board.unmakeMove (move, true);

    return moveNotation;
}

QString PGNCreator::GetSymbolFromPieceType (int pieceType) {
    switch (pieceType) {
    case Piece::Rook:
        return "R";
    case Piece::Knight:
        return "N";
    case Piece::Bishop:
        return "B";
    case Piece::Queen:
        return "Q";
    case Piece::King:
        return "K";
    default:
        return "";
    }
}
