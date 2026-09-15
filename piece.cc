#include "piece.h"

using namespace std;

Piece::Piece(PieceType pieceType, Colour colour) : pieceType{pieceType}, colour{colour}, moveCount{0} {}

PieceType Piece::getPieceType() const {
    return pieceType;
}

Colour Piece::getColour() const {
    return colour;
}

bool Piece::hasMoved() const {
    return (moveCount > 0);
}

void Piece::incrementMoveCount() {
    moveCount++;
}

bool Piece::operator==(const Piece& other) const{
   return (pieceType == other.pieceType && colour == other.colour && moveCount == other.moveCount);
}
