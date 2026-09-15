#ifndef STATE_H
#define STATE_H

#include "enumerated.h"
#include "position.h"

enum class StateType {EmptyCell, RelayEmptyCell, NewPiece, RelayNewPiece, Update, RelayUpdate, Reply, isPinnedCheck, isPinnedConfirm, replyProtected};
enum class Direction {NW, N, NE, W, E, SW, S, SE, KNIGHT};

struct State {

    StateType type = StateType::EmptyCell;
    Colour colour = Colour::NONE;         // What colour was the original notifying piece?
    PieceType pieceType = PieceType::NONE; // What type of piece is the original notifying piece?
    Position position{};                   // What is the position of the original notifying piece?
    Direction direction = Direction::N;    // Direction from the receiver to notifier

};

#endif
