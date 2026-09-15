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
    // True when getMove reads the squares from the terminal. A computer player does
    // not, so whatever the user typed after the command has to be thrown away.
    virtual bool usesTerminalInput() const;
    Colour getColour() const;
};

#endif
