#ifndef PLAYER_H
#define PLAYER_H
#include "move.h"
#include "enumerated.h"
#include "board.h"

class Player {
    Colour colour;

    public:
    Player(Colour colour);
    virtual ~Player() = default; // needed: players are owned through unique_ptr<Player>
    virtual Move getMove(Board *board) const = 0;
    Colour getColour() const;
};

#endif
