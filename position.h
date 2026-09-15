#ifndef POSITION_H
#define POSITION_H
#include <vector>
#include <iostream>

class Position{
    int row, col;

  public:
    // Default constructor
    Position();  

    // Constructor: takes column as character - converts to integer
    Position(int row, char col);
    Position(int row, int col);

    // Row accessor
    int getRow() const;
    int getRowVector() const;

    // Column accessors
    int getColInt() const;
    int getColVector() const;
    char getColChar() const;

    // Mutators
    void setColInt(int newC);
    void setRowInt(int newR);

    // True when this position refers to a real square (a1 - h8)
    bool isOnBoard() const;

    // Operator Overload
    bool operator==(const Position& other) const;

    friend std::ostream &operator<<(std::ostream &out, const Position &pos);

    static std::vector<Position> getAllPositions(Position from, Position to);
};

std::ostream &operator<<(std::ostream &out, const Position &pos);

#endif
