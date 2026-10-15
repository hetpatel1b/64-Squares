
#pragma once

#include "Position.hpp"

class Piece {
protected:
    Color color;

public:
    explicit Piece(Color c) : color(c) {}

    virtual ~Piece() = default;

    Color getColor() const {
        return color;
    }

    virtual PieceType getType() const = 0;

    // Checks movement geometry, not full move legality.
    virtual bool canMove(Position from, Position to) const = 0;
};
