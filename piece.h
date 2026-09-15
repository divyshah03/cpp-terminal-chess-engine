#ifndef PIECE_H
#define PIECE_H
#include "enumerated.h"
#include "position.h"
#include <vector>

class Piece {
    PieceType pieceType;
    Colour colour;
    int moveCount = 0;

    public:
    Piece(PieceType pieceType, Colour colour);
    PieceType getPieceType() const;
    Colour getColour() const;
    bool hasMoved() const;
    void incrementMoveCount();
    bool operator==(const Piece& other) const;
};

#endif
