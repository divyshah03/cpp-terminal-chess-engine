#ifndef HUMANPLAYER_H
#define HUMANPLAYER_H
#include "player.h"
#include "enumerated.h"
#include "move.h"
#include "board.h"

class HumanPlayer : public Player {
    public:
    HumanPlayer(Colour colour);
    Move getMove(Board *board) const override;
    bool usesTerminalInput() const override;
};

#endif
