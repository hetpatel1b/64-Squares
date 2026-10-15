
#pragma once
#include "Piece.hpp"

class Rook : public Piece {
public:
    explicit Rook(Color c) : Piece(c) {}

    PieceType getType() const override {
        return PieceType::Rook;
    }

    bool canMove(Position a, Position b) const override {
        return a.row == b.row || a.col == b.col;
    }
};
