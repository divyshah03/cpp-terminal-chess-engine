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
    Board();
    void init(std::vector<std::vector<char>> config);  // places the pieces and attaches displays
    bool movePiece(Move mv);
    bool undoMove();                                   // take back the last move

    bool isCheck() const;                              // is the side to move in check?
    bool isCheck(Colour side) const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool isSquareAttacked(Position pos, Colour attacker) const;

    // "What if" helpers used by the computer players. Each one plays the move on
    // a scratch copy of the position and puts everything back afterwards.
    bool wouldGiveCheck(const Move &mv);
    bool isAttackedAfter(const Move &mv, Position pos, Colour attacker);
    int opponentReplyCount(const Move &mv);

    void setCurrentTurn(Colour colour);
    Colour getCurrentTurn() const;
    void pushMove(Move mv);
    Move popMove();

    const std::vector<std::vector<Cell>>& getGrid() const;
    std::vector<Move> getBlackMoves() const;
    std::vector<Move> getWhiteMoves() const;
    std::vector<Move> getMoves(Colour side) const;
    Position getBKing() const;
    Position getWKing() const;

    void printTD() const;
};

#endif
