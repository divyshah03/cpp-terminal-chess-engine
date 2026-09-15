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

Move ComputerPlayer::getMove(Board *board) const {
    Move ret;
    vector<Move> validMoves;
    vector<Move> opponentMoves;
    vector<Position> positionsBetween;
    PieceType checker;
    bool blocker = false;
    int num = 0;

    if(getColour() == Colour::WHITE) {
        validMoves = board->getWhiteMoves();
        opponentMoves = board->getBlackMoves();
    } else {
        validMoves = board->getBlackMoves();
        opponentMoves = board->getWhiteMoves();
    }
    
    Position posKing;
    switch(level){
        case 1:
            srand(time(0));
            num = rand() % validMoves.size();
            ret = validMoves[num];
            break;

        case 3:

            // Prefers avoiding getting captured
            for(Move mv : validMoves) {
                for(Move attacked : opponentMoves) {
                    if(!(mv == attacked)) {
                        ret = mv;
                        break;
                    }
                }
            }

        case 2:

            //Prefers capturing moves
            for(Move mv : validMoves) {
                if(!(mv.getPieceType() == PieceType::NONE)) {
                    ret = mv;
                    break;
                }
            }

            // Prefers checks
            if(getColour() == Colour::WHITE)
                posKing = board->getBKing();
            else
                posKing = board->getWKing();

            for(Move mv : validMoves) {
                //checker piece type is the piece type of the attacker
                checker = board->getGrid()[mv.getFrom().getRowVector()][mv.getFrom().getColVector()].getPieceType();

                // If attacker is knight
                if(checker == PieceType::KNIGHT &&
                   ((abs(mv.getTo().getRowVector() - posKing.getRowVector()) == 2 && 
                    abs(mv.getTo().getColVector() - posKing.getColVector()) == 1) ||
                   (abs(mv.getTo().getRowVector() - posKing.getRowVector()) == 1 && 
                    abs(mv.getTo().getColVector() - posKing.getColVector()) == 2))) {
                    ret = mv;
                    break;
                }

                //If attacker is bishop
                if(checker == PieceType::BISHOP &&
                   abs(mv.getTo().getRowVector() - posKing.getRowVector()) == 
                   abs(mv.getTo().getColVector() - posKing.getColVector())) {
                    positionsBetween = Position::getAllPositions(mv.getTo(), posKing);

                    //Checking if there is a blocker
                    for(Position pos : positionsBetween) {
                        if(board->getGrid()[pos.getRowVector()][pos.getColVector()].getPieceType() != PieceType::NONE){
                            blocker = true;
                            break;
                        }
                    }
                    if(!blocker) {
                        ret = mv;
                        break;
                    }
                    
                    blocker = false;
                }

                // If attacker is a rook
                if(checker == PieceType::ROOK &&
                   ((mv.getTo().getRowVector() - posKing.getRowVector()) == 0 ||
                   (mv.getTo().getColVector() - posKing.getColVector()) == 0)) {
                    positionsBetween = Position::getAllPositions(mv.getTo(), posKing);

                    //Checking if there is a blocker
                    for(Position pos : positionsBetween) {
                        if(board->getGrid()[pos.getRowVector()][pos.getColVector()].getPieceType() != PieceType::NONE){
                            blocker = true;
                            break;
                        }
                    }
                    if(!blocker) {
                        ret = mv;
                        break;
                    }
                    
                    blocker = false;
                }

                if(checker == PieceType::QUEEN &&
                   (((mv.getTo().getRowVector() - posKing.getRowVector()) == 0 ||
                    (mv.getTo().getColVector() - posKing.getColVector()) == 0) ||
                   (abs(mv.getTo().getRowVector() - posKing.getRowVector()) == 
                    abs(mv.getTo().getColVector() - posKing.getColVector())))) {
                        positionsBetween = Position::getAllPositions(mv.getTo(), posKing);

                    //Checking if there is a blocker
                    for(Position pos : positionsBetween) {
                        if(board->getGrid()[pos.getRowVector()][pos.getColVector()].getPieceType() != PieceType::NONE){
                            blocker = true;
                            break;
                        }
                    }
                    if(!blocker) {
                        ret = mv;
                        break;
                    }
                    
                    blocker = false;
                }

                // Only other possible piece that can check is pawn
                if(getColour() == Colour::WHITE) {
                    if((posKing.getRow() - mv.getTo().getRow()) == 1 &&
                        abs(posKing.getColInt() - mv.getTo().getRow()) == 1) {
                            ret = mv;
                            break;
                    }
                }

                else if(getColour() == Colour::BLACK) {
                    if((mv.getTo().getRow() - posKing.getRow()) == 1 &&
                        abs(posKing.getColInt() - mv.getTo().getRow()) == 1) {
                            ret = mv;
                            break;
                    }
                }
            }

            // Play any move
            ret = validMoves[0];
            break;
    }
    return ret;
}
