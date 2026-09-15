#include "cell.h"
#include "position.h"
#include "piece.h"
#include "enumerated.h"
#include "state.h"
#include "move.h"

using namespace std;

// Default constructor - only for resizing in Board
Cell::Cell()
    : occupant{PieceType::NONE, Colour::NONE} {}

Cell::Cell(Position position, Piece occupant)
     : position{position}, occupant{occupant} {}

Position Cell::getPosition() const {
    return position;
}

Piece Cell::getPiece() const {
    return occupant;
}

void Cell::setPiece(Piece piece) {
    occupant = piece;
}

bool Cell::isOccupied() const {
    return (occupant.getPieceType() != PieceType::NONE);
}

Info Cell::getInfo() const {
    return Info{position, occupant.getColour(), occupant.getPieceType()};
}

PieceType Cell::getPieceType() const {
    return occupant.getPieceType();
}

Colour Cell::getColour() const {
    return occupant.getColour();
}

void Cell::setCell(Info info, State state) {
    position = info.getPosition();
    occupant = Piece{info.getPieceType(), info.getColour()};
    notifyObservers();
    setState(state);
}

PieceType Cell::getPieceType() const {
    return getPiece().getPieceType();
}

// Reverses the direction - helper function
Direction Cell::reverseDirection(Direction dir){
    switch(dir) {
        case Direction::N: return Direction::S;
        case Direction::S: return Direction::N;
        case Direction::E: return Direction::W;
        case Direction::W: return Direction::E;
        case Direction::NE: return Direction::SW;
        case Direction::NW: return Direction::SE;
        case Direction::SE: return Direction::NW;
        case Direction::SW: return Direction::NE;
        default: return Direction::KNIGHT; 
    }
}

bool Cell::Attackable(PieceType pieceType, Direction dir, Colour colour){
    vector<Direction> vectorToCheck;
    switch(pieceType){
        case(PieceType::NONE):
            vectorToCheck = NoneAttackDir;
            break;
        case(PieceType::KING):
            vectorToCheck = KingAttackDir;
            break;
        case(PieceType::QUEEN):
            vectorToCheck = QueenAttackDir;
            break;
        case(PieceType::ROOK):
            vectorToCheck = RookAttackDir;
            break;
        case(PieceType::BISHOP):
            vectorToCheck = BishopAttackDir;
            break;
        case(PieceType::KNIGHT):
            vectorToCheck = KnightAttackDir;
            break;
        case(PieceType::PAWN):
            vectorToCheck = colour == Colour::BLACK ? BPawnAttackDir : WPawnAttackDir;
            break;
    }

    for(const auto& d : vectorToCheck){
        if(d == dir) return true;
    }    

    return false;
}

bool Cell::moveAblePawn(Colour colour, Direction dir){
    if(colour == Colour::WHITE){
        return (dir == Direction::S);
    } else {
        return (dir == Direction::N);
    }
}

Direction Cell::calcDirection(Position pos1, Position pos2){
    int dr = pos1.getRowVector() - pos2.getRowVector();
    int dc = pos1.getColVector() - pos2.getColVector();

    Direction dir;

    // Cardinal/diagonal directions
    if (dr == -1 && dc == -1) dir = Direction::NW;
    else if (dr == -1 && dc == 0)  dir = Direction::N;
    else if (dr == -1 && dc == 1)  dir = Direction::NE;
    else if (dr == 0 && dc == -1)  dir = Direction::W;
    else if (dr == 0 && dc == 1)   dir = Direction::E;
    else if (dr == 1 && dc == -1)  dir = Direction::SW;
    else if (dr == 1 && dc == 0)   dir = Direction::S;
    else if (dr == 1 && dc == 1)   dir = Direction::SE;

    // Knight moves (L-shapes)
    else if ((abs(dr) == 2 && abs(dc) == 1) || (abs(dr) == 1 && abs(dc) == 2))
        dir = Direction::KNIGHT;

    return dir;
}

// Reset helper 
void Cell::Reset(){
    allValidMoves.clear();
    isProtected = false;
}

void Cell::NoneResponse(State fromState, Direction calculatedDir){
    switch(fromState.type){

        case StateType::EmptyCell:
            Reset();
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::RelayEmptyCell, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }
        
        case StateType::RelayEmptyCell:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::RelayEmptyCell, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            Reset();
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{StateType::Reply, getInfo().getColour(), PieceType::NONE, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Reply, getInfo().getColour(), PieceType::NONE, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();

                setState(State{StateType::RelayNewPiece, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::RelayNewPiece:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::RelayNewPiece, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            Reset();
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{StateType::Reply, getInfo().getColour(), PieceType::NONE, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Reply, getInfo().getColour(), PieceType::NONE, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();

                setState(State{StateType::RelayUpdate, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::RelayUpdate:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::RelayUpdate, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::isPinnedCheck:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::isPinnedCheck, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::isPinnedConfirm:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::isPinnedConfirm, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }

        case StateType::replyProtected:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::replyProtected, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            } 

        case StateType::Reply:
            Reset();
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Reply, fromState.colour, fromState.pieceType, fromState.position, calculatedDir});
                notifyObservers();
                break;
            }
    }
}

void Cell::KingResponse(State fromState, Direction calculatedDir){
    StateType replyType;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }
        
        case StateType::RelayEmptyCell:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers(); 
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            break;

        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }
            break;

        case StateType::isPinnedCheck:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else if(getInfo().getColour() == fromState.colour){ // the king must be the same colour 
                setState(State{StateType::isPinnedConfirm, getInfo().getColour(), PieceType::KING, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
                break;
            }

        case StateType::isPinnedConfirm:
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }

        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());

                // King distance check
                int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                canAttack = (canAttack && (dr < 2 && dc < 2));

                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::QueenResponse(State fromState, Direction calculatedDir){
    Direction revDir;
    StateType replyType;
    vector<Move> tempValidMoves;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }
        
        case StateType::RelayEmptyCell:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers(); 

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::isPinnedCheck:
            break;

        case StateType::isPinnedConfirm:
            revDir = reverseDirection(fromState.direction);
            for(const auto& mv: allValidMoves){
                Direction moveDirection = calcDirection(mv.getFrom(), mv.getTo());
                if(moveDirection == calculatedDir || moveDirection == revDir){
                    tempValidMoves.push_back(mv);
                }
            }

            allValidMoves.clear();
            allValidMoves = tempValidMoves;
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }

        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());

                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::BishopResponse(State fromState, Direction calculatedDir){
    Direction revDir;
    vector<Move> tempValidMoves;
    StateType replyType;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::RelayEmptyCell:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers(); 

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else { // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::BISHOP, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::isPinnedCheck:
            break;

        case StateType::isPinnedConfirm:
            revDir = reverseDirection(fromState.direction);
            for(const auto& mv: allValidMoves){
                Direction moveDirection = calcDirection(mv.getFrom(), mv.getTo());
                if(moveDirection == calculatedDir || moveDirection == revDir){
                    tempValidMoves.push_back(mv);
                }
            }

            allValidMoves.clear();
            allValidMoves = tempValidMoves;
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }

        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());

                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::KnightResponse(State fromState, Direction calculatedDir){
    Direction revDir;
    vector<Move> tempValidMoves;
    StateType replyType;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If NOT KNIGHT direction cell, nothing to do
            if(calculatedDir != Direction::KNIGHT){
                break;
            } else { // Direction is KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), getInfo().getPieceType(), getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::RelayEmptyCell:
            // If NOT KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else { // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else { // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::KNIGHT, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::isPinnedCheck:
            break;

        case StateType::isPinnedConfirm:
            revDir = reverseDirection(fromState.direction);
            for(const auto& mv: allValidMoves){
                Direction moveDirection = calcDirection(mv.getFrom(), mv.getTo());
                if(moveDirection == calculatedDir || moveDirection == revDir){
                    tempValidMoves.push_back(mv);
                }
            }

            allValidMoves.clear();
            allValidMoves = tempValidMoves;
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }
        
        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());

                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::RookResponse(State fromState, Direction calculatedDir){
    Direction revDir;
    vector<Move> tempValidMoves;
    StateType replyType;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else { // Direction is not KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::RelayEmptyCell:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;
        
        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::ROOK, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::isPinnedCheck:
            break;

        case StateType::isPinnedConfirm:
            revDir = reverseDirection(fromState.direction);
            for(const auto& mv: allValidMoves){
                Direction moveDirection = calcDirection(mv.getFrom(), mv.getTo());
                if(moveDirection == calculatedDir || moveDirection == revDir){
                    tempValidMoves.push_back(mv);
                }
            }

            allValidMoves.clear();
            allValidMoves = tempValidMoves;
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }

        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());

                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::PawnResponse(State fromState, Direction calculatedDir){
    Direction revDir;
    vector<Move> tempValidMoves;
    StateType replyType;
    switch(fromState.type){

        case StateType::EmptyCell:
            // If KNIGHT direction cell, nothing to do
            if(calculatedDir == Direction::KNIGHT){
                break;
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{StateType::Update, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), Direction::N});
                // setState direction is just a place holder in this case
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::RelayEmptyCell:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir == Direction::KNIGHT || calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{StateType::Update, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), Direction::N});
                // Direction::N is placeholder
                Reset(); // reset list of valid moves and protected status
                notifyObservers();
                break;
            }

        case StateType::NewPiece:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                setState(State{replyType, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }
            setState(State{StateType::Update, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), Direction::N});
            // Direction for update is placeholder
            Reset();
            notifyObservers();

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayNewPiece:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), reverseDirection(calculatedDir)});
            }

            setState(State{StateType::Update, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), Direction::N});
            // Direction::N is placeholder
            Reset(); // reset list of valid moves and protected status
            notifyObservers(); 

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::Update:
            // If KNIGHT direction cell, reply if sender is KNIGHT
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                break;
            } else if(calculatedDir != Direction::KNIGHT){ // Direction is not KNIGHT direction
                setState(State{replyType, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::RelayUpdate:
            replyType = fromState.colour == getInfo().getColour() ? StateType::replyProtected : StateType::Reply;
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction){
                break;
            } else { 
                setState(State{replyType, getInfo().getColour(), PieceType::PAWN, getInfo().getPosition(), reverseDirection(calculatedDir)});
                notifyObservers();
            }

            //isPinnedCheck is done after getting all its moves
            if(fromState.colour != getInfo().getColour() && fromState.pieceType != PieceType::KING && 
               fromState.pieceType != PieceType::PAWN && fromState.pieceType != PieceType::KNIGHT && 
               Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour)) {
                setState(State{StateType::isPinnedCheck, getInfo().getColour(), PieceType::QUEEN, getInfo().getPosition(), calculatedDir});
                notifyObservers();
            }
            break;

        case StateType::isPinnedCheck:
            break;

        case StateType::isPinnedConfirm:
            revDir = reverseDirection(fromState.direction);
            for(const auto& mv: allValidMoves){
                Direction moveDirection = calcDirection(mv.getFrom(), mv.getTo());
                if(moveDirection == calculatedDir || moveDirection == revDir){
                    tempValidMoves.push_back(mv);
                }
            }

            allValidMoves.clear();
            allValidMoves = tempValidMoves;
            break;

        case StateType::replyProtected:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(isProtected) break;
            if(calculatedDir != fromState.direction || fromState.colour != getPiece().getColour()){
                break;
            } else if(calculatedDir == Direction::KNIGHT && fromState.pieceType == PieceType::KNIGHT){
                isProtected = true;
            } else { 
                isProtected = Attackable(fromState.pieceType, reverseDirection(calculatedDir), fromState.colour);

                // check that the position is a neighbour if the piece is King or Pawn
                if(isProtected && (fromState.pieceType == PieceType::KING || fromState.pieceType == PieceType::PAWN)){
                    int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                    int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                    isProtected = (dr < 2 && dc < 2);
                }
            }

        case StateType::Reply:
            // If KNIGHT direction, nothing to do
            // If directions don't match, do nothing
            if(calculatedDir != fromState.direction || fromState.colour == getPiece().getColour()){
                break;
            } else { 
                bool canAttack = false;
                canAttack = Attackable(getPieceType(), calculatedDir, getPiece().getColour());
                // if the pawn is to move south or north accordingly
                canAttack = (canAttack || moveAblePawn(getInfo().getColour(), calculatedDir)); 
                
                // Pawn distance check
                int dr = abs(fromState.position.getRowVector() - getPosition().getRowVector());
                int dc = abs(fromState.position.getColVector() - getPosition().getColVector());
                canAttack = (canAttack && (dr < 2 && dc < 2));
                
                // First move - 2 cell move possibility
                if(moveAblePawn(getInfo().getColour(), calculatedDir) && fromState.pieceType == PieceType::NONE &&
                ((getInfo().getColour() == Colour::WHITE && getInfo().getPosition().getRowVector() == 1) || 
                (getInfo().getColour() == Colour::BLACK && getInfo().getPosition().getRowVector() == 6))){
                    canAttack = true;
                }
                
                if(canAttack){
                    Move mv{getInfo().getPosition(), fromState.position, fromState.pieceType};
                    allValidMoves.push_back(mv);
                }
            }
    }
}

void Cell::notify(Subject &whoFrom){
    Info myInfo = getInfo();
    Info fromInfo = whoFrom.getInfo();
    State fromState = whoFrom.getState();

    int dr = fromInfo.getPosition().getRowVector() - myInfo.getPosition().getRowVector();
    int dc = fromInfo.getPosition().getColVector() - myInfo.getPosition().getColVector();

    Direction dir;

    // Cardinal/diagonal directions
    if (dr == -1 && dc == -1) dir = Direction::NW;
    else if (dr == -1 && dc == 0)  dir = Direction::N;
    else if (dr == -1 && dc == 1)  dir = Direction::NE;
    else if (dr == 0 && dc == -1)  dir = Direction::W;
    else if (dr == 0 && dc == 1)   dir = Direction::E;
    else if (dr == 1 && dc == -1)  dir = Direction::SW;
    else if (dr == 1 && dc == 0)   dir = Direction::S;
    else if (dr == 1 && dc == 1)   dir = Direction::SE;

    // Knight moves (L-shapes)
    else if ((abs(dr) == 2 && abs(dc) == 1) || (abs(dr) == 1 && abs(dc) == 2))
        dir = Direction::KNIGHT;

    else
        return; // not a valid 8-direction or knight move

    switch(myInfo.getPieceType()){
        case PieceType::NONE : 
            this->NoneResponse(fromState, dir);
            break;
        case PieceType::KING : 
            KingResponse(fromState, dir);
            break;
        case PieceType::QUEEN : 
            QueenResponse(fromState, dir);
            break;
        case PieceType::ROOK : 
            RookResponse(fromState, dir);
            break;
        case PieceType::BISHOP : 
            BishopResponse(fromState, dir);
            break;
        case PieceType::KNIGHT : 
            KnightResponse(fromState, dir);
            break;
        case PieceType::PAWN : 
            PawnResponse(fromState, dir);
            break;
    }
}


