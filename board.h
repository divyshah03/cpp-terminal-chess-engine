#ifndef BOARD_H
#define BOARD_H
#include "cell.h"
#include "textDisplay.h"
#ifdef ENABLE_GRAPHICS
#include "graphicsDisplay.h"
#endif
#include "position.h"
#include "piece.h"
#include "move.h"
#include "enumerated.h"
#include <vector>
#include <memory>

class Board {
    std::vector<std::vector<Cell>> grid;
    std::vector<Move> movesPlayed;
    std::vector<Move> blackMoves;
    std::vector<Move> whiteMoves;
    std::unique_ptr<TextDisplay> td;   // Changed to unique_ptr
    //std::unique_ptr<GraphicsDisplay> gd; // Changed to unique_ptr
    Position posBKing;
    Position posWKing;
    Colour currentTurn;

    public:
    void init(std::vector<std::vector<char>> config);  // places the pieces on an empty board
                                                       // attach observers; neighbours plus displays
    bool movePiece(Move mv);

    bool isCheck();
    bool isCheckmate();
    bool isStalemate();

    void setCurrentTurn(Colour colour);
    Colour getCurrentTurn();
    void pushMove(Move mv);
    Move popMove();

    const std::vector<std::vector<Cell>>& getGrid();
    std::vector<Move> getBlackMoves();
    std::vector<Move> getWhiteMoves();
    Position getBKing();
    Position getWKing();

    void printTD();
};

#endif
