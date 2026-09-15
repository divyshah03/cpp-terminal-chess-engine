#include "humanPlayer.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

HumanPlayer::HumanPlayer(Colour colour) : Player{colour} {}

bool HumanPlayer::usesTerminalInput() const {
    return true;
}

// "e2" -> Position; returns an off-board position when the text is not a square
static Position parseSquare(const string &text) {
    if (text.length() != 2) return Position{};
    char file = tolower(static_cast<unsigned char>(text[0]));
    char rank = text[1];
    if (file < 'a' || file > 'h' || rank < '1' || rank > '8') return Position{};
    return Position{rank - '0', file};   // rank as a number, file as a character
}

// Optional fourth token of "move e7 e8 Q"
static PieceType parsePromotion(const string &text) {
    if (text.empty()) return PieceType::NONE;
    PieceType pt = pieceTypeFromChar(text[0]);
    if (pt == PieceType::QUEEN || pt == PieceType::ROOK ||
        pt == PieceType::BISHOP || pt == PieceType::KNIGHT) {
        return pt;
    }
    return PieceType::NONE;
}

Move HumanPlayer::getMove(Board *board) const {
    string pos;

    char col1, col2 = '@';
    char row1, row2 = -1;

    // From position
    cin >> col1;
    cin >> row1;

    // To position
    cin >> col2;
    cin >> row2;

    Position from{row1, col1};
    Position to{row2, col2};

    PieceType pt = board->getGrid()[to.getRowVector()][to.getColVector()].getPieceType();
    Move mv{from, to, pt};
    return mv;
}
