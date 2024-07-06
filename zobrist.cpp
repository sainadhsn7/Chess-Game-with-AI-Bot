#include "zobrist.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>

//static members allocation
// const QString Zobrist::randomNumbersFileName = "RandomNumbers.txt";
// const QString Zobrist::randomNumbersPath =  QCoreApplication::applicationDirPath() + "/" + randomNumbersFileName;
// QVector<QVector<QVector<uint64_t>>> Zobrist:: piecesArray = QVector<QVector<QVector<uint64_t>>>(8, QVector<QVector<uint64_t>>(2, QVector<uint64_t>(64)));
// QVector<uint64_t> Zobrist:: castlingRights = QVector<uint64_t>(16);
// QVector<uint64_t> Zobrist:: enPassantFile = QVector<uint64_t>(9);

Zobrist::Zobrist(){
    randomNumbersFileName = "RandomNumbers.txt";
    randomNumbersPath =  QCoreApplication::applicationDirPath() + "/" + randomNumbersFileName;
    piecesArray = QVector<QVector<QVector<uint64_t>>>(8, QVector<QVector<uint64_t>>(2, QVector<uint64_t>(64)));
    castlingRights = QVector<uint64_t>(16);
    enPassantFile = QVector<uint64_t>(9);
    auto randomNumbers = readRandomNumbers ();
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        for (int pieceIndex = 0; pieceIndex < 8; pieceIndex++) {
        	piecesArray[pieceIndex][Board::WhiteIndex][squareIndex] = randomNumbers.dequeue ();
            piecesArray[pieceIndex][Board::BlackIndex][squareIndex] = randomNumbers.dequeue ();
        }
    }
    
    for (int i = 0; i < 16; i++) {
        castlingRights[i] = randomNumbers.dequeue ();
    }
    
    for (int i = 0; i < enPassantFile.size(); i++) {
        enPassantFile[i] = randomNumbers.dequeue ();
    }
    
    sideToMove = randomNumbers.dequeue ();
}

uint64_t Zobrist:: calculateZobristKey(Board *board){
    uint64_t zobristKey = 0;
    Piece Piece;
    for (int squareIndex = 0; squareIndex < 64; squareIndex++) {
        if (board->Square[squareIndex] != 0) {
            int pieceType = Piece.PieceType (board->Square[squareIndex]);
            int pieceColour = Piece.Colour (board->Square[squareIndex]);
            
            zobristKey ^= piecesArray[pieceType][(pieceColour == Piece.White) ? Board:: WhiteIndex : Board:: BlackIndex][squareIndex];
        }
    }
    
    int epIndex = (int) (board->currentGameState >> 4) & 15;
    if (epIndex != -1) {
        zobristKey ^= enPassantFile[epIndex];
    }
    
    if (board->ColourToMove == Piece.Black) {
        zobristKey ^= sideToMove;
    }
    
    zobristKey ^= castlingRights[board->currentGameState & 0b1111];
    
    return zobristKey;
}

uint64_t Zobrist:: randomUnsigned64BitNumber() {
    return std::uniform_int_distribution<uint64_t>()(prng);
}

void Zobrist::writeRandomNumbers() {
    prng.seed(seed);
    QString randomNumberString;
    int numRandomNumbers = 64 * 8 * 2 + castlingRights.size() + enPassantFile.size() + 1;
    
    for (int i = 0; i < numRandomNumbers; ++i) {
        randomNumberString += QString::number(randomUnsigned64BitNumber());
        if (i != numRandomNumbers - 1) {
            randomNumberString += ',';
        }
    }
    QFile outFile(randomNumbersPath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&outFile);
        out << randomNumberString;
        outFile.close();
    }
}

QQueue<uint64_t> Zobrist::readRandomNumbers() {
    QQueue<uint64_t> randomNumbers;
    QFile inFile(randomNumbersPath);
    if (!inFile.exists()) {
        qDebug() << "writing random numbers in file \n";
        writeRandomNumbers();
    }
    
    if (inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&inFile);
        QString numbersString = in.readAll();
        inFile.close();
        
        QStringList numberStrings = numbersString.split(',');
        
        for (const QString &numberString : numberStrings) {
            randomNumbers.push_back(numberString.toULongLong());
        }
    } else {
       qDebug() << "Could not open file for reading: " << randomNumbersPath.toStdString() << "\n";
    }
    return randomNumbers;
}
