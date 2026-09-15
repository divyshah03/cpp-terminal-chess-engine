#ifndef COMPUTERPLAYER_H
#define COMPUTERPLAYER_H

#include "player.h"
#include "move.h"
#include "board.h"
#include "enumerated.h"
#include <string>

class ComputerPlayer : public Player {
    int level;
    int scoreMove(Board *board, const Move &mv) const;

  public:
    ComputerPlayer(Colour colour, int level);
    Move getMove(Board *board) const override;
    int getLevel();
};

#endif
