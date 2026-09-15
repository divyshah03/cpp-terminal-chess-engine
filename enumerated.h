#ifndef ENUMERATED_H
#define ENUMERATED_H

#include <iostream>

enum class Colour{
    WHITE,
    BLACK,
    NONE,
    GREY,
    GREEN,
    LIGHTBROWN,
    DARKBROWN,
    PINK,
    RED,
    BLUE,
    ORANGE
};

enum class PieceType{
    KING,
    QUEEN,
    BISHOP,
    ROOK,
    KNIGHT,
    PAWN,
    NONE
};

// How a move has to be executed on the board
enum class MoveType{
    NORMAL,
    CASTLE_KINGSIDE,
    CASTLE_QUEENSIDE,
    ENPASSANT,
    PROMOTION
};

// The colour that is not the given colour; NONE for anything that is not a side
Colour opponentOf(Colour colour);

// Rough material worth of a piece, used by the computer players
int pieceValue(PieceType pieceType);

// Board configuration characters: uppercase is white, lowercase is black
PieceType pieceTypeFromChar(char ch);
Colour colourFromChar(char ch);

// Output operator for PieceType
std::ostream& operator<<(std::ostream &out, PieceType pt);

#endif
