#include "player.h"

Player::Player(Colour colour) : colour{colour} {}

bool Player::usesTerminalInput() const {
    return false;
}

Colour Player::getColour() const {
    return colour;
}
