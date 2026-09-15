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
    std::vector<Move> blackMoves;   // fully legal moves for black
    std::vector<Move> whiteMoves;   // fully legal moves for white
    std::unique_ptr<TextDisplay> td;
#ifdef ENABLE_GRAPHICS
    std::unique_ptr<GraphicsDisplay> gd;
#endif
    Position posBKing;
    Position posWKing;
    Colour currentTurn = Colour::WHITE;
    Position epTarget;  // square an enemy pawn may capture onto (en passant)
    Position epVictim;  // square of the pawn that may be captured that way

    // Full position, used for take-back and for testing move legality
    struct Snapshot {
        std::vector<std::vector<Piece>> pieces;
        Position posBKing;
        Position posWKing;
        Position epTarget;
        Position epVictim;
        Colour currentTurn;
    };
    std::vector<Snapshot> history;

    static bool inBounds(int row, int col);
    Piece pieceAt(int row, int col) const;
    bool isEmpty(int row, int col) const;
    void setPieceAt(int row, int col, Piece piece);  // does not notify observers

    Snapshot snapshot() const;
    void restore(const Snapshot &snap);
    void applyMove(const Move &mv, bool countMoves);  // no validation
    void refreshDisplay();

    // Move generation
    std::vector<Move> pseudoLegalMoves(Colour side) const;
    void addOffsetMoves(int row, int col, Colour side,
                        const std::vector<std::pair<int,int>> &offsets,
                        std::vector<Move> &moves) const;
    void addSlidingMoves(int row, int col, Colour side,
                         const std::vector<std::pair<int,int>> &directions,
                         std::vector<Move> &moves) const;
    void addPawnMoves(int row, int col, Colour side, std::vector<Move> &moves) const;
    void addPawnMove(int fromRow, int fromCol, int toRow, int toCol,
                     PieceType captured, Colour side, std::vector<Move> &moves) const;
    void addCastlingMoves(int row, int col, Colour side, std::vector<Move> &moves) const;
    std::vector<Move> legalMoves(Colour side);
    void generateAllMoves();

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
