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
    string line, fromText, toText, promoText;

    // The rest of the line after the "move" command holds both squares
    while (getline(cin, line)) {
        istringstream iss{line};
        if (iss >> fromText >> toText) {
            iss >> promoText;
            break;
        }
        cout << "Please enter both squares, for example: e2 e4" << endl;
        fromText.clear();
        toText.clear();
    }

    Position from = parseSquare(fromText);
    Position to = parseSquare(toText);
    if (!from.isOnBoard() || !to.isOnBoard()) {
        cout << "That is not a square on the board." << endl;
        return Move{};   // invalid move; Game rejects it
    }

    PieceType captured = board->getGrid()[to.getRowVector()][to.getColVector()].getPieceType();
    return Move{from, to, captured, MoveType::NORMAL, parsePromotion(promoText)};
}
