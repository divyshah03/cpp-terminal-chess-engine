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
    // Clear existing grid and observers (to prevent dangling pointers)
    // If a grid already exists, we must detach its observers before replacing td/gd
    for (auto &row : grid) {
        for (auto &cell : row) {
            cell.detachAll();
        }
    }

    grid.clear();
    grid.resize(GRID_SIZE, std::vector<Cell>(GRID_SIZE));

    // Create displays
    td = std::make_unique<TextDisplay>(GRID_SIZE);
    //gd = std::make_unique<GraphicsDisplay>(GRID_SIZE);

    //Converting each char to info to use setCell
    for(int i = 0; i < GRID_SIZE ;i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            char ch = config[i][j];
            Position pos{i, j};
            Colour col;
            PieceType pt;

            if(ch > 'A' && ch < 'Z')
                col = Colour::WHITE;
            else if (ch > 'a' && ch < 'z')
                col = Colour::BLACK;
            else 
                col = Colour::NONE;

            if(ch == 'k'){
                pt = PieceType::KING;
                posBKing = pos;
            } else if(ch == 'K'){
                pt = PieceType::KING;
                posWKing = pos;
            }else if(ch == 'q' || ch == 'Q')
                pt = PieceType::QUEEN;
            else if(ch == 'b' || ch == 'B')
                pt = PieceType::BISHOP;
            else if(ch == 'n' || ch == 'N')
                pt = PieceType::KNIGHT;
            else if(ch == 'r' || ch == 'R')
                pt = PieceType::ROOK;
            else if(ch == 'p' || ch == 'P')
                pt = PieceType::PAWN;
            else
                pt = PieceType::NONE;

            Info inf{pos, col, pt};
            State state = (pt == PieceType::NONE) ? State{StateType::EmptyCell, Colour::NONE, PieceType::NONE, pos, Direction::N}
                                                : State{StateType::Update, col, pt, pos, Direction::N};
            grid[pos.getRowVector()][pos.getColVector()].setCell(inf, state);
            
            // Attach both displays as observers
            grid[pos.getRowVector()][pos.getColVector()].attach(td.get());
            //grid[pos.getRowVector()][pos.getColVector()].attach(gd.get());
        }
    }

    // Knight move directions: (row offset, column offset)
    const std::vector<std::pair<int, int>> knightMoves = {
        {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
        {-2, -1}, {-1, -2}, {1, -2}, {2, -1}
    };

    for (size_t row = 0; row < GRID_SIZE; ++row) {
        for (size_t column = 0; column < GRID_SIZE; ++column) {
            // Attach adjacent 8 neighbors
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    int nr = row + dr;
                    int nc = column + dc;
                    if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                        grid[row][column].attach(&grid[nr][nc]); // Attach in only one direction
                    }
                }
            }

            // Attach knight-move neighbors
            for (const auto& [dr, dc] : knightMoves) {
                int nr = row + dr;
                int nc = column + dc;
                if (nr >= 0 && nr < GRID_SIZE && nc >= 0 && nc < GRID_SIZE) {
                    grid[row][column].attach(&grid[nr][nc]);
                }
            }
        }
    }

    // Set initial states for all cells
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            // to ensure each piece has state update or empty cell as appropriate
            State state = (grid[i][j].getInfo().getPieceType() == PieceType::NONE) ? 
                        State{StateType::EmptyCell, Colour::NONE, PieceType::NONE, grid[i][j].getInfo().getPosition(), Direction::N}
                                                : State{StateType::Update, grid[i][j].getInfo().getColour(), 
                                                    grid[i][j].getInfo().getPieceType(), grid[i][j].getInfo().getPosition(), Direction::N};
            grid[i][j].setState(state);
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
