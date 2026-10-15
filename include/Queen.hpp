
#pragma once
#include "Piece.hpp"
#include <cstdlib>

class Queen : public Piece {
public:
    explicit Queen(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::Queen;
    }

    bool canMove(Position a, Position b) const override {
        int r = std::abs(a.row - b.row);
        int c = std::abs(a.col - b.col);

        return a.row == b.row || a.col == b.col ||
               r == c;
    }
};
