#ifndef COORD_H
#define COORD_H

class Coord{
public:
    int fileIndex;
    int rankIndex;

    Coord (int rankIndex, int fileIndex) {
        this->fileIndex = fileIndex;
        this->rankIndex = rankIndex;
    }

    bool IsLightSquare () {
        return (fileIndex + rankIndex) % 2 != 0;
    }

    bool CompareTo (Coord other) {
        return (fileIndex == other.fileIndex && rankIndex == other.rankIndex) ? 1 : 0;
    }
};

#endif // COORD_H
