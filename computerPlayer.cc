#include "computerPlayer.h"
#include "position.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

using namespace std;

// Constructor
ComputerPlayer::ComputerPlayer(Colour colour, int level)
    : Player{colour}, level{level} {}

// Level accessor - returns int
int ComputerPlayer::getLevel() {
    return level;
}

// Scores a move from the point of view of this player. Higher is better.
// Level 1 does not score at all, it plays at random.
int ComputerPlayer::scoreMove(Board *board, const Move &mv) const {
    Colour opponent = opponentOf(getColour());
    int score = 0;

    // Level 2 and up: take material, and take the most valuable piece available
    if (level >= 2 && mv.isCaptured()) {
        score += 10 * pieceValue(mv.getPieceType());
    }

    // Promotions are almost always worth playing
    if (level >= 2 && mv.getMoveType() == MoveType::PROMOTION) {
        score += 8 * pieceValue(mv.getPromotion());
    }

    // Level 2 and up: prefer moves that give check
    if (level >= 2 && board->wouldGiveCheck(mv)) {
        score += 12;
    }

    // Level 3 and up: avoid moving onto a square the opponent attacks,
    // and weigh the risk by the value of the piece being moved
    if (level >= 3) {
        PieceType mover = board->getGrid()[mv.getFrom().getRowVector()][mv.getFrom().getColVector()].getPieceType();
        if (board->isAttackedAfter(mv, mv.getTo(), opponent)) {
            score -= 9 * pieceValue(mover);
        }
    }

    // Level 4: also step away from pieces that are attacked right now,
    // and prefer moves that leave the opponent with fewer replies
    if (level >= 4) {
        PieceType mover = board->getGrid()[mv.getFrom().getRowVector()][mv.getFrom().getColVector()].getPieceType();
        if (board->isSquareAttacked(mv.getFrom(), opponent)) {
            score += 4 * pieceValue(mover);
        }
        score -= board->opponentReplyCount(mv);
    }

    return score;
}

Move ComputerPlayer::getMove(Board *board) const {
    vector<Move> validMoves = board->getMoves(getColour());
    if (validMoves.empty()) return Move{};   // no legal move; Game reports it as invalid

    if (level <= 1) {
        static bool seeded = false;
        if (!seeded) {                       // seed once, not on every move
            srand(static_cast<unsigned>(time(nullptr)));
            seeded = true;
        }
        return validMoves[rand() % validMoves.size()];
    }

    Move best = validMoves[0];
    int bestScore = scoreMove(board, best);
    for (size_t i = 1; i < validMoves.size(); ++i) {
        int score = scoreMove(board, validMoves[i]);
        if (score > bestScore) {
            bestScore = score;
            best = validMoves[i];
        }
    }
    return best;
}
