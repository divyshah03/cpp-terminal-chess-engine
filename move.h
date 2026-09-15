#ifndef MOVE_H
#define MOVE_H
#include "position.h"
#include "enumerated.h"

class Move {
    Position from;
    Position to;
    PieceType pieceCaptured;
    MoveType type;
    PieceType promotion;

    public:
    Move(); // Default constructor that creates an invalid move
    Move(Position from, Position to, PieceType pieceCaptured,
         MoveType type = MoveType::NORMAL, PieceType promotion = PieceType::NONE);
    Position getFrom() const;
    Position getTo() const;
    PieceType getPieceType() const;   // the piece this move captures
    MoveType getMoveType() const;
    PieceType getPromotion() const;
    bool isCaptured() const;
    bool isValid() const;             // both squares are on the board
    bool sameSquares(const Move& other) const;
    bool operator==(const Move& other) const;
};

#endif
