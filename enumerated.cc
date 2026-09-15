#include "enumerated.h"
#include <cctype>
#include <iostream>

using namespace std;

Colour opponentOf(Colour colour){
    if(colour == Colour::WHITE) return Colour::BLACK;
    if(colour == Colour::BLACK) return Colour::WHITE;
    return Colour::NONE;
}

int pieceValue(PieceType pieceType){
    switch(pieceType){
        case PieceType::QUEEN:  return 9;
        case PieceType::ROOK:   return 5;
        case PieceType::BISHOP: return 3;
        case PieceType::KNIGHT: return 3;
        case PieceType::PAWN:   return 1;
        case PieceType::KING:   return 100;
        case PieceType::NONE:   return 0;
    }
    return 0;
}

PieceType pieceTypeFromChar(char ch){
    switch(tolower(static_cast<unsigned char>(ch))){
        case 'k': return PieceType::KING;
        case 'q': return PieceType::QUEEN;
        case 'b': return PieceType::BISHOP;
        case 'r': return PieceType::ROOK;
        case 'n': return PieceType::KNIGHT;
        case 'p': return PieceType::PAWN;
        default:  return PieceType::NONE;
    }
}

Colour colourFromChar(char ch){
    if(pieceTypeFromChar(ch) == PieceType::NONE) return Colour::NONE;
    if(isupper(static_cast<unsigned char>(ch))) return Colour::WHITE;
    return Colour::BLACK;
}

std::ostream& operator<<(std::ostream &out, PieceType pt){
    switch(pt){
        case PieceType::KING:   return out << "King";
        case PieceType::QUEEN:  return out << "Queen";
        case PieceType::BISHOP: return out << "Bishop";
        case PieceType::ROOK:   return out << "Rook";
        case PieceType::KNIGHT: return out << "Knight";
        case PieceType::PAWN:   return out << "Pawn";
        case PieceType::NONE:   return out << "None";
        default:                return out << "Should not reach this case";
    }
}
