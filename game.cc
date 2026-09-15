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

    if(player1 == "human"){
        whitePlayer = make_unique<HumanPlayer>(Colour::WHITE);
    } else if(player1 == "computer1"){
        whitePlayer = make_unique<ComputerPlayer>(Colour::WHITE, 1);
    } else if(player1 == "computer2"){
        whitePlayer = make_unique<ComputerPlayer>(Colour::WHITE, 2);
    } else if(player1 == "computer3"){
        whitePlayer = make_unique<ComputerPlayer>(Colour::WHITE, 3);
    } else{
        whitePlayer = make_unique<ComputerPlayer>(Colour::WHITE, 4);
    }

    if(player2 == "human"){
        blackPlayer = make_unique<HumanPlayer>(Colour::BLACK);
    } else if(player2 == "computer1"){
        blackPlayer = make_unique<ComputerPlayer>(Colour::BLACK, 1);
    } else if(player2 == "computer2"){
        blackPlayer = make_unique<ComputerPlayer>(Colour::BLACK, 2);
    } else if(player2 == "computer3"){
        blackPlayer = make_unique<ComputerPlayer>(Colour::BLACK, 3);
    } else{
        blackPlayer = make_unique<ComputerPlayer>(Colour::BLACK, 4);
    } 

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

bool Game::isValidMove(Move move) {
    Player *player = getCurrentTurn();

    if(player->getColour() == Colour::WHITE){
        for(const auto& m : getWhiteMoves()){
            if(m == move){
                return true;
            }
        }
    } else if(player->getColour() == Colour::BLACK){
        for(const auto& m : getBlackMoves()){
            if(m == move){
                return true;
            }
        }
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

    if(tempBoard.isCheck()){
        return false;
    }
    return true;
}


bool Game::gameMove() {
    Move mv = currentTurn->getMove(getBoard());
    if(isValidMove(mv)){
        cout << (board->getGrid())[mv.getFrom().getRowVector()][mv.getFrom().getColVector()].getPieceType() <<" moved from " << mv.getFrom() << " to " << mv.getTo() << endl;
        return board->movePiece(mv);
    } else {
        return false;
    }

    // Print textdisplay
    board->printTD();
}
