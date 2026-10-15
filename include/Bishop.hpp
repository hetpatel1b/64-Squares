
#pragma once
#include "Piece.hpp"
#include <cstdlib>

class Bishop : public Piece {
public:
    explicit Bishop(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::Bishop;
    }

    bool canMove(Position a, Position b) const override {
        return std::abs(a.row - b.row) ==
               std::abs(a.col - b.col);
    }
};
