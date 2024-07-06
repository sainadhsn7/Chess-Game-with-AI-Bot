#ifndef MOVE_H
#define MOVE_H

#include "piece.h"
#include "BoardRepresentation.h"
class Move
{
public:
    struct Flag {
        static const int None = 0;
        static const int EnPassantCapture = 1;
        static const int Castling = 2;
        static const int PromoteToQueen = 3;
        static const int PromoteToKnight = 4;
        static const int PromoteToRook = 5;
        static const int PromoteToBishop = 6;
        static const int PawnTwoForward = 7;
    };

    short moveValue;

    const short startSquareMask = 0b0000000000111111;
    const short targetSquareMask = 0b0000111111000000;
    const short flagMask = 0b1111000000000000;

    Move (short moveValue) {
        this->moveValue = moveValue;
    }

    Move (int startSquare, int targetSquare) {
        moveValue = (short) (startSquare | targetSquare << 6);
    }

    Move (int startSquare, int targetSquare, int flag) {
        moveValue = (short) (startSquare | targetSquare << 6 | flag << 12);
    }

    // Copy constructor
    Move(const Move& other) : moveValue(other.moveValue) {}

    // Move constructor
    Move(Move&& other) noexcept : moveValue(other.moveValue) {
        other.moveValue = 0;
    }

    // Copy assignment operator
    Move& operator=(const Move& other) {
        if (this != &other) {
            this->moveValue = other.moveValue;
        }
        return *this;
    }

    // Move assignment operator
    Move& operator=(Move&& other) noexcept {
        if (this != &other) {
            this->moveValue = other.moveValue;
            other.moveValue = 0;
        }
        return *this;
    }

    int StartSquare(){
            return moveValue & startSquareMask;
    }

    int TargetSquare(){
            return (moveValue & targetSquareMask) >> 6;
    }

    bool IsPromotion(){
        int flag = MoveFlag();
        return flag == Flag::PromoteToQueen || flag == Flag::PromoteToRook || flag == Flag::PromoteToKnight || flag == Flag::PromoteToBishop;
    }

    int MoveFlag(){
            return moveValue >> 12;
    }

    int PromotionPieceType() {
        if(MoveFlag() == Flag::PromoteToRook)
            return Piece::Rook;
        else if(MoveFlag() == Flag::PromoteToKnight)
            return Piece::Knight;
        else if(MoveFlag() == Flag::PromoteToBishop)
            return Piece::Bishop;
        else if(MoveFlag() == Flag::PromoteToQueen)
            return Piece::Queen;
        else return Piece::None;
    }

    static Move InvalidMove() {
        return  Move(0);
    }

    static bool SameMove (Move a, Move b) {
        return a.moveValue == b.moveValue;
    }

    short Value() {
        return moveValue;
    }

    bool IsInvalid() {
        return moveValue == 0;

    }

    QString Name() {
        return BoardRepresentation::SquareNameFromIndex (StartSquare()) + "-" + BoardRepresentation::SquareNameFromIndex (TargetSquare());
    }
};

#endif // MOVE_H
