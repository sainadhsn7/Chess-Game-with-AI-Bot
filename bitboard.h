#ifndef BITBOARD_H
#define BITBOARD_H

#include <cstdint>
class BitBoard
{
public:
    static bool ContainsSquare (uint64_t bitboard, int square) {
        return ((bitboard >> square) & 1) != 0;
    }
    BitBoard();
};

#endif // BITBOARD_H
