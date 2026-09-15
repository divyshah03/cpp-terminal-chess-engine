#include <iostream>
#include <memory>
#include <string>
#include "game.h"
#include "humanPlayer.h"
#include "computerPlayer.h"
#include "move.h"

using namespace std;

const vector<vector<char>> Game::DEFAULT_CONFIG = {
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}, 
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}, 
    {' ', '_', ' ', '_', ' ', '_', ' ', '_'}, 
    {'_', ' ', '_', ' ', '_', ' ', '_', ' '}, 
    {' ', '_', ' ', '_', ' ', '_', ' ', '_'}, 
    {'_', ' ', '_', ' ', '_', ' ', '_', ' '}, 
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}, 
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}  
};

Game::Game()
    : config{DEFAULT_CONFIG} {}

double Game::getWhiteWins() {
    return whiteWins;
}
double Game::getBlackWins() {
    return blackWins;
}   
void Game::incrementWhiteWins(double value) {
    whiteWins += value;
}
void Game::incrementBlackWins(double value) {
    blackWins += value;
}

Player *Game::getCurrentTurn() {
    return currentTurn;
}

Player *Game::getWhitePlayer() {
    return whitePlayer.get();
}

Player *Game::getBlackPlayer() {
    return blackPlayer.get();
}

Board *Game::getBoard() {
    return board.get();
}

bool Game::isStarted() const {
    return (board != nullptr && currentTurn != nullptr);
}

vector<Move> Game::getBlackMoves() {
    return board->getBlackMoves();
}

vector<Move> Game::getWhiteMoves() {
    return board->getWhiteMoves();
}

// Creates a player from a command line name; unknown names become a level 1 computer
static unique_ptr<Player> makePlayer(const string &name, Colour colour) {
    if (name == "human")     return make_unique<HumanPlayer>(colour);
    if (name == "computer2") return make_unique<ComputerPlayer>(colour, 2);
    if (name == "computer3") return make_unique<ComputerPlayer>(colour, 3);
    if (name == "computer4") return make_unique<ComputerPlayer>(colour, 4);
    return make_unique<ComputerPlayer>(colour, 1);
}

void Game::start(string player1, string player2, Colour colour) {

    // Reset previous state
    whitePlayer.reset();
    blackPlayer.reset();
    board.reset();
    currentTurn = nullptr;

    whitePlayer = makePlayer(player1, Colour::WHITE);
    blackPlayer = makePlayer(player2, Colour::BLACK);

    if(colour == Colour::WHITE){
        currentTurn = getWhitePlayer();
    } else{
        currentTurn = getBlackPlayer();
    }

    board = make_unique<Board>();
    board->setCurrentTurn(colour);
    board->init(config);

    // Print textdisplay
    board->printTD();
}

void Game::switchTurn() {
    currentTurn = (currentTurn == getWhitePlayer()) ? getBlackPlayer() : getWhitePlayer();
}

bool Game::findLegalMove(const Move &requested, Move &found) {
    if (!isStarted() || !requested.isValid()) return false;

    vector<Move> candidates = board->getMoves(currentTurn->getColour());
    const Move *fallback = nullptr;

    for (const Move &candidate : candidates) {
        if (!candidate.sameSquares(requested)) continue;

        // A promotion has four possible moves for the same pair of squares
        if (candidate.getMoveType() == MoveType::PROMOTION) {
            if (requested.getPromotion() != PieceType::NONE) {
                if (candidate.getPromotion() == requested.getPromotion()) {
                    found = candidate;
                    return true;
                }
                continue;
            }
            if (candidate.getPromotion() == PieceType::QUEEN) {  // queen by default
                found = candidate;
                return true;
            }
        }

        if (fallback == nullptr) fallback = &candidate;
    }

    if (fallback != nullptr) {
        found = *fallback;
        return true;
    }
    return false;
}

bool Game::isSetupValid() {
    int whiteKingCount = 0;
    int blackKingCount = 0;

    // Check for pawns on invalid first/last rows
    for (int i = 0; i < 8; ++i) {
        if (config[0][i] == 'p' || config[0][i] == 'P' ||
            config[7][i] == 'p' || config[7][i] == 'P') {
            return false;
        }
    }    

    // Check for correct king count
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){

            if(config[i][j] == 'K'){
                whiteKingCount++;
            } else if(config[i][j] == 'k'){
                blackKingCount++;
            }
        }
    }

    if(whiteKingCount != 1 || blackKingCount != 1){
        return false;
    }

    Board tempBoard;
    tempBoard.init(config);

    // Neither king may already be under attack
    if(tempBoard.isCheck(Colour::WHITE) || tempBoard.isCheck(Colour::BLACK)){
        return false;
    }
    return true;
}


bool Game::gameMove() {
    if (!isStarted()) return false;

    Move requested = currentTurn->getMove(getBoard());
    Move mv;
    if (!findLegalMove(requested, mv)) return false;

    PieceType moved = board->getGrid()[mv.getFrom().getRowVector()][mv.getFrom().getColVector()].getPieceType();
    if (!board->movePiece(mv)) return false;

    cout << moved << " moved from " << mv.getFrom() << " to " << mv.getTo();
    switch (mv.getMoveType()) {
        case MoveType::CASTLE_KINGSIDE:  cout << " (castled kingside)"; break;
        case MoveType::CASTLE_QUEENSIDE: cout << " (castled queenside)"; break;
        case MoveType::ENPASSANT:        cout << " (en passant)"; break;
        case MoveType::PROMOTION:        cout << " (promoted to " << mv.getPromotion() << ")"; break;
        case MoveType::NORMAL:
            if (mv.isCaptured()) cout << " (captured " << mv.getPieceType() << ")";
            break;
    }
    cout << endl;

    switchTurn();

    // Print textdisplay
    board->printTD();
    return true;
}

bool Game::undoMove() {
    if (!isStarted()) return false;
    if (!board->undoMove()) return false;

    switchTurn();
    board->printTD();
    return true;
}
