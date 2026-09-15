#include "cell.h"
#include "position.h"
#include "piece.h"
#include "enumerated.h"
#include "state.h"
#include "move.h"

using namespace std;

// Default constructor - only for resizing in Board
Cell::Cell()
    : occupant{PieceType::NONE, Colour::NONE} {}

Cell::Cell(Position position, Piece occupant)
     : position{position}, occupant{occupant} {}

Position Cell::getPosition() const {
    return position;
}

Piece Cell::getPiece() const {
    return occupant;
}

void Cell::setPiece(Piece piece) {
    occupant = piece;
}

bool Cell::isOccupied() const {
    return (occupant.getPieceType() != PieceType::NONE);
}

Info Cell::getInfo() const {
    return Info{position, occupant.getColour(), occupant.getPieceType()};
}

PieceType Cell::getPieceType() const {
    return occupant.getPieceType();
}

Colour Cell::getColour() const {
    return occupant.getColour();
}

void Cell::setCell(Info info, State state) {
    position = info.getPosition();
    occupant = Piece{info.getPieceType(), info.getColour()};
    setState(state);
    notifyObservers();
}
