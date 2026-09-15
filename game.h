#ifndef GAME_H
#define GAME_H
#include "board.h"
#include "player.h"
#include "move.h"
#include <vector>
#include <memory>
#include <string>

class Game {
    std::unique_ptr<Board> board;
    Player *currentTurn = nullptr; // can stay as Player * since it does not own the Player object
    std::unique_ptr<Player> whitePlayer;
    std::unique_ptr<Player> blackPlayer;
    double whiteWins = 0;
    double blackWins = 0;
    // Finds the generated legal move that matches what was requested, so that
    // castling, en passant and promotion are executed correctly
    bool findLegalMove(const Move &requested, Move &found);
    void switchTurn();
    static const std::vector<std::vector<char>> DEFAULT_CONFIG;

    public:
    Game();
    std::vector<std::vector<char>> config;
    double getWhiteWins();
    double getBlackWins();
    void incrementWhiteWins(double value);
    void incrementBlackWins(double value);
    Player *getCurrentTurn();
    Player *getWhitePlayer();
    Player *getBlackPlayer();
    Board *getBoard();
    std::vector<Move> getBlackMoves();
    std::vector<Move> getWhiteMoves();
    void start(std::string player1, std::string player2, Colour colour);
    bool isSetupValid();
    bool gameMove();
};

#endif
