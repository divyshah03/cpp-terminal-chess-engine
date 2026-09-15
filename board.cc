#include "board.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>

using namespace std;

static const int GRID_SIZE = 8;

static const vector<pair<int,int>> KNIGHT_OFFSETS = {
    {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
};
static const vector<pair<int,int>> KING_OFFSETS = {
    {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
};
static const vector<pair<int,int>> ROOK_DIRECTIONS = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1}
};
static const vector<pair<int,int>> BISHOP_DIRECTIONS = {
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};

Board::Board() : currentTurn{Colour::WHITE} {}

bool Board::inBounds(int row, int col) {
    return (row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE);
}

Piece Board::pieceAt(int row, int col) const {
    return grid[row][col].getPiece();
}

bool Board::isEmpty(int row, int col) const {
    return (pieceAt(row, col).getPieceType() == PieceType::NONE);
}

void Board::setPieceAt(int row, int col, Piece piece) {
    grid[row][col].setPiece(piece);
}

void Board::init(vector<vector<char>> config) {
    // Detach the observers of any previous grid before the displays are replaced
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell.detachAll();
        }
    }

    grid.clear();
    grid.resize(GRID_SIZE, vector<Cell>(GRID_SIZE));

    movesPlayed.clear();
    history.clear();
    whiteMoves.clear();
    blackMoves.clear();
    epTarget = Position{};
    epVictim = Position{};
    posWKing = Position{};
    posBKing = Position{};

    // Create displays
    td = make_unique<TextDisplay>(GRID_SIZE);
#ifdef ENABLE_GRAPHICS
    gd = make_unique<GraphicsDisplay>(GRID_SIZE);
#endif

    // Give every cell its position and attach the displays first, so that the
    // notifications sent while the pieces are placed actually reach them
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            Position pos{row, col};
            grid[row][col] = Cell{pos, Piece{PieceType::NONE, Colour::NONE}};
            grid[row][col].attach(td.get());
#ifdef ENABLE_GRAPHICS
            grid[row][col].attach(gd.get());
#endif
        }
    }

    // Convert each configuration character into a piece
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            char ch = config[row][col];
            PieceType pt = pieceTypeFromChar(ch);
            Colour colour = colourFromChar(ch);
            Position pos{row, col};

            if (pt == PieceType::KING) {
                if (colour == Colour::WHITE) posWKing = pos;
                else posBKing = pos;
            }

            State state = (pt == PieceType::NONE)
                ? State{StateType::EmptyCell, Colour::NONE, PieceType::NONE, pos, Direction::N}
                : State{StateType::NewPiece, colour, pt, pos, Direction::N};
            grid[row][col].setCell(Info{pos, colour, pt}, state);
        }
    }

    generateAllMoves();
}

void Board::refreshDisplay() {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            grid[row][col].notifyObservers();
        }
    }
}

Board::Snapshot Board::snapshot() const {
    Snapshot snap;
    snap.pieces.reserve(GRID_SIZE);
    for (int row = 0; row < GRID_SIZE; ++row) {
        vector<Piece> pieceRow;
        pieceRow.reserve(GRID_SIZE);
        for (int col = 0; col < GRID_SIZE; ++col) {
            pieceRow.push_back(pieceAt(row, col));
        }
        snap.pieces.push_back(std::move(pieceRow));
    }
    snap.posBKing = posBKing;
    snap.posWKing = posWKing;
    snap.epTarget = epTarget;
    snap.epVictim = epVictim;
    snap.currentTurn = currentTurn;
    return snap;
}

void Board::restore(const Snapshot &snap) {
    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            setPieceAt(row, col, snap.pieces[row][col]);
        }
    }
    posBKing = snap.posBKing;
    posWKing = snap.posWKing;
    epTarget = snap.epTarget;
    epVictim = snap.epVictim;
    currentTurn = snap.currentTurn;
}

// Executes a move without checking it. Callers must pass a generated move.
void Board::applyMove(const Move &mv, bool countMoves) {
    int fromRow = mv.getFrom().getRowVector();
    int fromCol = mv.getFrom().getColVector();
    int toRow = mv.getTo().getRowVector();
    int toCol = mv.getTo().getColVector();

    Piece mover = pieceAt(fromRow, fromCol);
    if (countMoves) mover.incrementMoveCount();

    // En passant: the captured pawn sits beside the origin square, not on the target
    if (mv.getMoveType() == MoveType::ENPASSANT) {
        setPieceAt(fromRow, toCol, Piece{PieceType::NONE, Colour::NONE});
    }

    setPieceAt(fromRow, fromCol, Piece{PieceType::NONE, Colour::NONE});

    Piece placed = mover;
    if (mv.getMoveType() == MoveType::PROMOTION) {
        placed = Piece{mv.getPromotion(), mover.getColour()};
        if (countMoves) placed.incrementMoveCount();
    }
    setPieceAt(toRow, toCol, placed);

    // Castling also moves the rook
    if (mv.getMoveType() == MoveType::CASTLE_KINGSIDE) {
        Piece rook = pieceAt(fromRow, GRID_SIZE - 1);
        if (countMoves) rook.incrementMoveCount();
        setPieceAt(fromRow, GRID_SIZE - 1, Piece{PieceType::NONE, Colour::NONE});
        setPieceAt(fromRow, toCol - 1, rook);
    } else if (mv.getMoveType() == MoveType::CASTLE_QUEENSIDE) {
        Piece rook = pieceAt(fromRow, 0);
        if (countMoves) rook.incrementMoveCount();
        setPieceAt(fromRow, 0, Piece{PieceType::NONE, Colour::NONE});
        setPieceAt(fromRow, toCol + 1, rook);
    }

    // Track the kings
    if (placed.getPieceType() == PieceType::KING) {
        if (placed.getColour() == Colour::WHITE) posWKing = mv.getTo();
        else posBKing = mv.getTo();
    }

    // A double pawn step creates an en passant opportunity for one move only
    if (mover.getPieceType() == PieceType::PAWN && abs(toRow - fromRow) == 2) {
        epTarget = Position{(toRow + fromRow) / 2, toCol};
        epVictim = mv.getTo();
    } else {
        epTarget = Position{};
        epVictim = Position{};
    }
}

bool Board::isSquareAttacked(Position pos, Colour attacker) const {
    if (!pos.isOnBoard() || grid.empty()) return false;
    int row = pos.getRowVector();
    int col = pos.getColVector();

    // Knights
    for (const auto &[dr, dc] : KNIGHT_OFFSETS) {
        int r = row + dr, c = col + dc;
        if (!inBounds(r, c)) continue;
        Piece p = pieceAt(r, c);
        if (p.getColour() == attacker && p.getPieceType() == PieceType::KNIGHT) return true;
    }

    // Enemy king on an adjacent square
    for (const auto &[dr, dc] : KING_OFFSETS) {
        int r = row + dr, c = col + dc;
        if (!inBounds(r, c)) continue;
        Piece p = pieceAt(r, c);
        if (p.getColour() == attacker && p.getPieceType() == PieceType::KING) return true;
    }

    // Pawns: white pawns capture towards higher rows, so they stand one row below
    int pawnRow = (attacker == Colour::WHITE) ? row - 1 : row + 1;
    for (int dc : {-1, 1}) {
        int c = col + dc;
        if (!inBounds(pawnRow, c)) continue;
        Piece p = pieceAt(pawnRow, c);
        if (p.getColour() == attacker && p.getPieceType() == PieceType::PAWN) return true;
    }

    // Rooks and queens along ranks and files
    for (const auto &[dr, dc] : ROOK_DIRECTIONS) {
        int r = row + dr, c = col + dc;
        while (inBounds(r, c)) {
            Piece p = pieceAt(r, c);
            if (p.getPieceType() != PieceType::NONE) {
                if (p.getColour() == attacker &&
                    (p.getPieceType() == PieceType::ROOK || p.getPieceType() == PieceType::QUEEN))
                    return true;
                break;
            }
            r += dr;
            c += dc;
        }
    }

    // Bishops and queens along diagonals
    for (const auto &[dr, dc] : BISHOP_DIRECTIONS) {
        int r = row + dr, c = col + dc;
        while (inBounds(r, c)) {
            Piece p = pieceAt(r, c);
            if (p.getPieceType() != PieceType::NONE) {
                if (p.getColour() == attacker &&
                    (p.getPieceType() == PieceType::BISHOP || p.getPieceType() == PieceType::QUEEN))
                    return true;
                break;
            }
            r += dr;
            c += dc;
        }
    }

    return false;
}

void Board::addOffsetMoves(int row, int col, Colour side,
                           const vector<pair<int,int>> &offsets, vector<Move> &moves) const {
    for (const auto &[dr, dc] : offsets) {
        int r = row + dr, c = col + dc;
        if (!inBounds(r, c)) continue;
        Piece target = pieceAt(r, c);
        if (target.getColour() == side) continue;  // cannot capture our own piece
        moves.push_back(Move{Position{row, col}, Position{r, c}, target.getPieceType()});
    }
}

void Board::addSlidingMoves(int row, int col, Colour side,
                            const vector<pair<int,int>> &directions, vector<Move> &moves) const {
    for (const auto &[dr, dc] : directions) {
        int r = row + dr, c = col + dc;
        while (inBounds(r, c)) {
            Piece target = pieceAt(r, c);
            if (target.getPieceType() == PieceType::NONE) {
                moves.push_back(Move{Position{row, col}, Position{r, c}, PieceType::NONE});
            } else {
                if (target.getColour() != side) {
                    moves.push_back(Move{Position{row, col}, Position{r, c}, target.getPieceType()});
                }
                break;  // the ray stops at the first piece
            }
            r += dr;
            c += dc;
        }
    }
}

// Adds one pawn move, expanding it into the four promotion choices when needed
void Board::addPawnMove(int fromRow, int fromCol, int toRow, int toCol,
                        PieceType captured, Colour side, vector<Move> &moves) const {
    int promoRow = (side == Colour::WHITE) ? GRID_SIZE - 1 : 0;
    Position from{fromRow, fromCol};
    Position to{toRow, toCol};

    if (toRow == promoRow) {
        for (PieceType promo : {PieceType::QUEEN, PieceType::ROOK,
                                PieceType::BISHOP, PieceType::KNIGHT}) {
            moves.push_back(Move{from, to, captured, MoveType::PROMOTION, promo});
        }
    } else {
        moves.push_back(Move{from, to, captured});
    }
}

void Board::addPawnMoves(int row, int col, Colour side, vector<Move> &moves) const {
    int dir = (side == Colour::WHITE) ? 1 : -1;
    int startRow = (side == Colour::WHITE) ? 1 : GRID_SIZE - 2;
    int oneStep = row + dir;

    // Straight ahead, only onto an empty square
    if (inBounds(oneStep, col) && isEmpty(oneStep, col)) {
        addPawnMove(row, col, oneStep, col, PieceType::NONE, side, moves);

        int twoStep = row + 2 * dir;
        if (row == startRow && inBounds(twoStep, col) && isEmpty(twoStep, col)) {
            moves.push_back(Move{Position{row, col}, Position{twoStep, col}, PieceType::NONE});
        }
    }

    // Diagonals, only onto an enemy piece or an en passant square
    for (int dc : {-1, 1}) {
        int c = col + dc;
        if (!inBounds(oneStep, c)) continue;
        Piece target = pieceAt(oneStep, c);

        if (target.getPieceType() != PieceType::NONE) {
            if (target.getColour() != side) {
                addPawnMove(row, col, oneStep, c, target.getPieceType(), side, moves);
            }
        } else if (epTarget.isOnBoard() && epVictim.isOnBoard() &&
                   epTarget == Position{oneStep, c}) {
            Piece victim = pieceAt(epVictim.getRowVector(), epVictim.getColVector());
            if (victim.getPieceType() == PieceType::PAWN && victim.getColour() != side) {
                moves.push_back(Move{Position{row, col}, Position{oneStep, c},
                                     PieceType::PAWN, MoveType::ENPASSANT});
            }
        }
    }
}

void Board::addCastlingMoves(int row, int col, Colour side, vector<Move> &moves) const {
    int homeRow = (side == Colour::WHITE) ? 0 : GRID_SIZE - 1;
    if (row != homeRow || col != 4) return;              // king must be on its original square
    if (pieceAt(row, col).hasMoved()) return;
    Colour attacker = opponentOf(side);
    if (isSquareAttacked(Position{row, col}, attacker)) return;  // cannot castle out of check

    // Kingside: rook on h-file, f and g empty, king does not cross an attacked square
    Piece kingsideRook = pieceAt(homeRow, GRID_SIZE - 1);
    if (kingsideRook.getPieceType() == PieceType::ROOK && kingsideRook.getColour() == side &&
        !kingsideRook.hasMoved() && isEmpty(homeRow, 5) && isEmpty(homeRow, 6) &&
        !isSquareAttacked(Position{homeRow, 5}, attacker) &&
        !isSquareAttacked(Position{homeRow, 6}, attacker)) {
        moves.push_back(Move{Position{homeRow, 4}, Position{homeRow, 6},
                             PieceType::NONE, MoveType::CASTLE_KINGSIDE});
    }

    // Queenside: rook on a-file, b, c and d empty
    Piece queensideRook = pieceAt(homeRow, 0);
    if (queensideRook.getPieceType() == PieceType::ROOK && queensideRook.getColour() == side &&
        !queensideRook.hasMoved() && isEmpty(homeRow, 1) && isEmpty(homeRow, 2) &&
        isEmpty(homeRow, 3) &&
        !isSquareAttacked(Position{homeRow, 3}, attacker) &&
        !isSquareAttacked(Position{homeRow, 2}, attacker)) {
        moves.push_back(Move{Position{homeRow, 4}, Position{homeRow, 2},
                             PieceType::NONE, MoveType::CASTLE_QUEENSIDE});
    }
}

vector<Move> Board::pseudoLegalMoves(Colour side) const {
    vector<Move> moves;
    if (grid.empty()) return moves;

    for (int row = 0; row < GRID_SIZE; ++row) {
        for (int col = 0; col < GRID_SIZE; ++col) {
            Piece piece = pieceAt(row, col);
            if (piece.getColour() != side || piece.getPieceType() == PieceType::NONE) continue;

            switch (piece.getPieceType()) {
                case PieceType::PAWN:
                    addPawnMoves(row, col, side, moves);
                    break;
                case PieceType::KNIGHT:
                    addOffsetMoves(row, col, side, KNIGHT_OFFSETS, moves);
                    break;
                case PieceType::KING:
                    addOffsetMoves(row, col, side, KING_OFFSETS, moves);
                    addCastlingMoves(row, col, side, moves);
                    break;
                case PieceType::ROOK:
                    addSlidingMoves(row, col, side, ROOK_DIRECTIONS, moves);
                    break;
                case PieceType::BISHOP:
                    addSlidingMoves(row, col, side, BISHOP_DIRECTIONS, moves);
                    break;
                case PieceType::QUEEN:
                    addSlidingMoves(row, col, side, ROOK_DIRECTIONS, moves);
                    addSlidingMoves(row, col, side, BISHOP_DIRECTIONS, moves);
                    break;
                case PieceType::NONE:
                    break;
            }
        }
    }
    return moves;
}

// Keeps only the moves that do not leave (or place) our own king in check.
// This is what makes pins, blocks and forced king moves work.
vector<Move> Board::legalMoves(Colour side) {
    vector<Move> result;
    if (grid.empty()) return result;

    Snapshot snap = snapshot();
    for (const Move &mv : pseudoLegalMoves(side)) {
        applyMove(mv, false);
        if (!isCheck(side)) result.push_back(mv);
        restore(snap);
    }
    return result;
}

void Board::generateAllMoves() {
    whiteMoves = legalMoves(Colour::WHITE);
    blackMoves = legalMoves(Colour::BLACK);
}

bool Board::movePiece(Move mv) {
    if (grid.empty() || !mv.isValid()) return false;

    history.push_back(snapshot());
    movesPlayed.push_back(mv);

    applyMove(mv, true);

    currentTurn = opponentOf(currentTurn);

    refreshDisplay();
    generateAllMoves();
    return true;
}

bool Board::undoMove() {
    if (history.empty()) return false;

    restore(history.back());
    history.pop_back();
    if (!movesPlayed.empty()) movesPlayed.pop_back();

    refreshDisplay();
    generateAllMoves();
    return true;
}

bool Board::isCheck(Colour side) const {
    Position king = (side == Colour::WHITE) ? posWKing : posBKing;
    return isSquareAttacked(king, opponentOf(side));
}

bool Board::isCheck() const {
    return isCheck(currentTurn);
}

bool Board::wouldGiveCheck(const Move &mv) {
    if (grid.empty() || !mv.isValid()) return false;
    Colour mover = pieceAt(mv.getFrom().getRowVector(), mv.getFrom().getColVector()).getColour();
    if (mover == Colour::NONE) return false;

    Snapshot snap = snapshot();
    applyMove(mv, false);
    bool result = isCheck(opponentOf(mover));
    restore(snap);
    return result;
}

bool Board::isAttackedAfter(const Move &mv, Position pos, Colour attacker) {
    if (grid.empty() || !mv.isValid()) return false;

    Snapshot snap = snapshot();
    applyMove(mv, false);
    bool result = isSquareAttacked(pos, attacker);
    restore(snap);
    return result;
}

int Board::opponentReplyCount(const Move &mv) {
    if (grid.empty() || !mv.isValid()) return 0;
    Colour mover = pieceAt(mv.getFrom().getRowVector(), mv.getFrom().getColVector()).getColour();
    if (mover == Colour::NONE) return 0;

    Snapshot snap = snapshot();
    applyMove(mv, false);
    int result = static_cast<int>(pseudoLegalMoves(opponentOf(mover)).size());
    restore(snap);
    return result;
}

bool Board::isCheckmate() const {
    return (isCheck() && getMoves(currentTurn).empty());
}

bool Board::isStalemate() const {
    return (!isCheck() && getMoves(currentTurn).empty());
}

const vector<vector<Cell>>& Board::getGrid() const {
    return grid;
}

void Board::setCurrentTurn(Colour colour) {
    currentTurn = colour;
}

Colour Board::getCurrentTurn() const {
    return currentTurn;
}

void Board::pushMove(Move mv) {
    movesPlayed.push_back(mv);
}

Move Board::popMove() {
    if (movesPlayed.empty()) return Move{};
    Move last = movesPlayed.back();
    movesPlayed.pop_back();
    return last;
}

vector<Move> Board::getBlackMoves() const {
    return blackMoves;
}

vector<Move> Board::getWhiteMoves() const {
    return whiteMoves;
}

vector<Move> Board::getMoves(Colour side) const {
    return (side == Colour::WHITE) ? whiteMoves : blackMoves;
}

Position Board::getBKing() const {
    return posBKing;
}

Position Board::getWKing() const {
    return posWKing;
}

void Board::printTD() const {
    if (td) cout << *td << endl;
}
