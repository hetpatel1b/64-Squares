
#pragma once
#include "Piece.hpp"
#include <cstdlib>

class Pawn : public Piece {
public:
    explicit Pawn(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::Pawn;
    }

    bool canMove(Position a, Position b) const override {
        int dir = (color == Color::White) ? -1 : 1;
        int r = b.row - a.row;
        int c = std::abs(b.col - a.col);

        if (r == dir && c <= 1)
            return true;

        int start = (color == Color::White) ? 6 : 1;
        return a.row == start && r == 2 * dir && c == 0;
    }
};
