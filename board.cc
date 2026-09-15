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

    whiteMoves.clear();
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (grid[i][j].getPiece().getColour() == Colour::WHITE) {
                auto valid = grid[i][j].getAllValidMoves();
                whiteMoves.insert(whiteMoves.end(), valid.begin(), valid.end());
            }
        }
    }

    blackMoves.clear();
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (grid[i][j].getPiece().getColour() == Colour::BLACK) {
                auto valid = grid[i][j].getAllValidMoves();
                blackMoves.insert(blackMoves.end(), valid.begin(), valid.end());
            }
        }
    }
}

bool Board::movePiece(Move mv) {
    movesPlayed.push_back(mv); // update moves played
    Position from = mv.getFrom();
    Position to = mv.getTo();

    // Change king position if necessary
    if(from == posBKing){
        posBKing = to;
    } else if (from == posWKing){
        posWKing = to;
    }

    Piece capturingPiece = grid[from.getRowVector()][from.getColVector()].getPiece(); // moving piece
    capturingPiece.incrementMoveCount();

    // Setting from and to piece types and states
    grid[from.getRowVector()][from.getColVector()].setPiece(Piece{PieceType::NONE, Colour::NONE});
    grid[from.getRowVector()][from.getColVector()].setState(State{StateType::EmptyCell, Colour::NONE, PieceType::NONE, from, Direction::N});

    grid[to.getRowVector()][to.getColVector()].setPiece(capturingPiece);
    grid[to.getRowVector()][to.getColVector()].setState(State{StateType::NewPiece, capturingPiece.getColour(), 
                                                        capturingPiece.getPieceType(), to, Direction::N});
    
    //Notify rest
    grid[from.getRowVector()][from.getColVector()].notifyObservers();
    grid[to.getRowVector()][to.getColVector()].notifyObservers();

    // Change turn
    currentTurn = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;

    whiteMoves.clear();
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            if(grid[i][j].getPiece().getColour() == Colour::WHITE) {
                whiteMoves.insert(whiteMoves.end(), grid[i][j].getAllValidMoves().begin(), grid[i][j].getAllValidMoves().end());
            }
        }
    }

    blackMoves.clear();
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            if(grid[i][j].getPiece().getColour() == Colour::BLACK) {
                blackMoves.insert(blackMoves.end(), grid[i][j].getAllValidMoves().begin(), grid[i][j].getAllValidMoves().end());
                cout << "Move inserted - black" << endl;
            }
        }
    }
    
    return true; // Returns true if movePiece successful
}

bool Board::isCheck() {
    //Going through a list to see if the piece attacked is ever a king
    if(currentTurn == Colour::WHITE) {
        for(Move mv : blackMoves) {
            if(mv.isCaptured() && mv.getPieceType() == PieceType::KING)
                return true;
        }
    }
    else {
        for(Move mv : whiteMoves) {
            if(mv.isCaptured() && mv.getPieceType() == PieceType::KING)
                return true;
        }
    }
    return false;
}

bool Board::isCheckmate() {
    vector<Move> checkingMoves; // list of moves that are check
    Colour checker;
    if(currentTurn == Colour::WHITE) {
        checker = Colour::BLACK; // this colour did the check - challenger
        for(Move mv : blackMoves) {
            if(mv.getPieceType() == PieceType::KING) {
                checkingMoves.push_back(mv);
            }
        }
    }
    else {
        checker = Colour::WHITE;
        for(Move mv : whiteMoves) {
            if(mv.getPieceType() == PieceType::KING) {
                checkingMoves.push_back(mv);
            }
        }
    }

    //If no check, checkmate is false
    if(checkingMoves.size() == 0)
        return false;

    vector<Move> kingValidMoves;
    if(checker == Colour::WHITE) // checker is the attacker
        kingValidMoves = grid[posBKing.getRowVector()][posBKing.getColVector()].getAllValidMoves();
    else
        kingValidMoves = grid[posWKing.getRowVector()][posWKing.getColVector()].getAllValidMoves();

    //If more than one checker, king has to move
    if(checkingMoves.size() > 1) {

        // Now since we know the only valid moves are the kings moves, we can update the list of 
        // valid moves accordingly
        if(checker == Colour::WHITE) {
            blackMoves.clear();
            blackMoves = kingValidMoves;
        }
        else {
            whiteMoves.clear();
            whiteMoves = kingValidMoves;
        }

        //Checking Checkmate condition
        if(checker == Colour::WHITE &&
           (kingValidMoves.size() == 0))
            return true;
            
        if(checker == Colour::BLACK &&
           (kingValidMoves.size() == 0))
            return true;
        
        // More than one attacker and king has space to move
        return false;
    }


    //If knight is checker, king has to move (Only one knight can check at a time)
    Move check = checkingMoves[0];
    Position from = check.getFrom();
    Position to = check.getTo();
    if ((abs(to.getRow() - from.getRow()) == 2 && abs(to.getColInt() - from.getColInt()) == 1) ||
    (abs(to.getRow() - from.getRow()) == 1 && abs(to.getColInt() - from.getColInt()) == 2)) {

        vector<Move> tempValidMoves;

        //Black King has no moves
        if(checker == Colour::WHITE &&
           kingValidMoves.size() == 0) {
            for(Move mv : blackMoves) {
                if(check.getFrom() == mv.getTo()) {
                    tempValidMoves.push_back(mv);
                }
            }
            if(tempValidMoves.size() > 0) {
                blackMoves.clear();
                blackMoves = tempValidMoves;
                return false;
            } else {
                return true;
            }
           }
        
        //White King has no moves
        else if(checker == Colour::BLACK &&
           (kingValidMoves.size() == 0)) {
            for(Move mv : whiteMoves) {
                if(check.getFrom() == mv.getTo()) {
                    tempValidMoves.push_back(mv);
                }
            }
            if(tempValidMoves.size() > 0) {
                whiteMoves.clear();
                whiteMoves = tempValidMoves;
                return false;
            } else {
                return true;
            }
           }

        //Black King does have moves
        else if(checker == Colour::WHITE) {
            for(Move mv : blackMoves) {
                if(check.getFrom() == mv.getTo()) {
                    tempValidMoves.push_back(mv);
                }
            }

            //Adding king's valid moves to the list
            tempValidMoves.insert(tempValidMoves.end(), kingValidMoves.begin(), kingValidMoves.end());
            blackMoves.clear();
            blackMoves = tempValidMoves;
            return false;
        }

        //White King does have moves
        else {
            for(Move mv : whiteMoves) {
                if(check.getFrom() == mv.getTo()) {
                    tempValidMoves.push_back(mv);
                }
            }

            //Adding king's valid moves to the list
            tempValidMoves.insert(tempValidMoves.end(), kingValidMoves.begin(), kingValidMoves.end());
            whiteMoves.clear();
            whiteMoves = tempValidMoves;
            return false;
        }
    }

    vector<Move> tempValidMoves;

    // Adding the moves that capture the attacker 
    if(checker == Colour::WHITE){
        for(Move mv: blackMoves){
            if(check.getFrom() == mv.getTo()) {
                tempValidMoves.push_back(mv);
            }
        }
    } else {
        for(Move mv: whiteMoves){
            if(check.getFrom() == mv.getTo()) {
                tempValidMoves.push_back(mv);
            }
        }
    }

    // check is still the only move checking it. So we find all positions between the from and to
    // positions of this move and check which valid move can get in the way (including capture)
    vector<Position> positions = Position::getAllPositions(from, to);

    // Now positions contains all the list of positions where it can be blocked.
    // Hence all moves where a piece can enter this square is valid
    if(checker == Colour::BLACK) {
        for(Move mv : whiteMoves) {
            for(Position pos : positions) {
                if(mv.getTo() == pos)
                    tempValidMoves.push_back(mv);
            }
        }
    } else {
        for(Move mv : blackMoves) {
            for(Position pos : positions) {
                if(mv.getTo() == pos)
                    tempValidMoves.push_back(mv);
            }
        } 
    }

    // Adding the King's valid moves
    tempValidMoves.insert(tempValidMoves.end(), kingValidMoves.begin(), kingValidMoves.end());

    //Checking if there are any valid moves for that side to play;
    if(tempValidMoves.size() == 0)
        return true;
    else {
        if(checker == Colour::WHITE) {
            blackMoves.clear();
            blackMoves = tempValidMoves;
        }
        else {
            whiteMoves.clear();
            whiteMoves = tempValidMoves;
        }
        return false;
    }

}

bool Board::isStalemate() {
    if(currentTurn == Colour::WHITE)
        return (0 == whiteMoves.size());
    else
        return (0 == blackMoves.size());
}

const vector<vector<Cell>>& Board::getGrid(){
    return grid;
}

void Board::setCurrentTurn(Colour colour) {
    currentTurn = colour;
}

Colour Board::getCurrentTurn() {
    return currentTurn;
}

void Board::pushMove(Move mv) {
    movesPlayed.push_back(mv);
}

Move Board::popMove() {
    int last = movesPlayed.size() - 1;
    movesPlayed.pop_back();
    return movesPlayed[last];
}

vector<Move> Board::getBlackMoves() {
    return blackMoves;
}

vector<Move> Board::getWhiteMoves() {
    return whiteMoves;
}

Position Board::getBKing() {
    return posBKing;
}

Position Board::getWKing() {
    return posWKing;
}

void Board::printTD(){
    cout << *td << endl;
}
