#include "position.h"
#include <vector>
#include <iostream>

using namespace std;

// Default constructor
Position::Position()
    :row{-1}, col{-1} {}

// Constructor: takes column as character - converts to integer
Position::Position(int row, char col)
    : row{row} {
    int col_int = col - 'a' + 1; // column values start from 1; similar to row
    this->col = col_int;
}

Position::Position(int row, int col) : row{row + 1}, col{col + 1} {}

// Row accessor
int Position::getRow() const{
    return row;
}

// Row accessor - adjusts row value to be used in a vector
int Position::getRowVector() const{
    return getRow() - 1;
}

// Column accessor - returns column as int
int Position::getColInt() const{
    return col;
}

// Column accessor - returns column row adjusted to be used in a vector
int Position::getColVector() const{
    return col - 1;
}

// Column accessor - returns column as char
char Position::getColChar() const{
    return 'a' + col - 1;
}

// Column mutator
void Position::setColInt(int newC){
    col = newC;
}

// Row mutator
void Position::setRowInt(int newR){
    row = newR;
}

// True when this position refers to a real square (a1 - h8)
bool Position::isOnBoard() const{
    return (row >= 1 && row <= 8 && col >= 1 && col <= 8);
}

bool Position::operator==(const Position& other) const{
    return (row == other.getRow() && col == other.getColInt());
}

ostream &operator<<(std::ostream &out, const Position &pos){
    out << pos.getColChar() << pos.getRow();
    return out;
}

vector<Position> Position::getAllPositions(Position from, Position to) {
    //Bresenham algorithm
    // Finding all positions in between attacker (not knight) and king
    vector<Position> positions;

    int x0 = from.getColVector();
    int y0 = from.getRowVector();
    int x1 = to.getColVector();
    int y1 = to.getRowVector();

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;
    int x = x0;
    int y = y0;

    while (!(x == x1 && y == y1)) {
        // Move along the line
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }

        Position current{y, x};
        if (!(current == from || current == to)) {
            positions.push_back(current);
        }
    }    
    return positions;
}
