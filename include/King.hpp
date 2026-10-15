
#pragma once
#include "Piece.hpp"
#include <cstdlib>

class King : public Piece {
public:
    explicit King(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::King;
    }

    bool canMove(Position a, Position b) const override {
        int r = std::abs(a.row - b.row);
        int c = std::abs(a.col - b.col);

        return r <= 1 && c <= 1 && (r + c > 0);
    }
};
