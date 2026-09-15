#include "move.h"

// Default constructor - creates invalid move
Move::Move()
    : from{Position{}}, to{Position{}}, pieceCaptured{PieceType::NONE},
      type{MoveType::NORMAL}, promotion{PieceType::NONE} {}

Move::Move(Position from, Position to, PieceType pieceCaptured, MoveType type, PieceType promotion)
     : from{from}, to{to}, pieceCaptured{pieceCaptured}, type{type}, promotion{promotion} {}

Position Move::getFrom() const {
    return from;
}

Position Move::getTo() const {
    return to;
}

PieceType Move::getPieceType() const {
    return pieceCaptured;
}

MoveType Move::getMoveType() const {
    return type;
}

PieceType Move::getPromotion() const {
    return promotion;
}

bool Move::isCaptured() const {
    return (pieceCaptured != PieceType::NONE);
}

bool Move::operator==(const Move& other) const{
   return (from == other.from && to == other.to && pieceCaptured == other.pieceCaptured);
}
