
#pragma once
#include "Piece.hpp"
#include <cstdlib>

class Knight : public Piece {
public:
    explicit Knight(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::Knight;
    }

    bool canMove(Position a, Position b) const override {
        int r = std::abs(a.row - b.row);
        int c = std::abs(a.col - b.col);
        return (r == 2 && c == 1) ||
               (r == 1 && c == 2);
    }
};
