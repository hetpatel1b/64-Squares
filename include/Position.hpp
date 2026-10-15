
#pragma once

enum class Color {
    White,
    Black
};

enum class PieceType {
    Pawn,
    Rook,
    Knight,
    Bishop,
    Queen,
    King
};

struct Position {
    int row;
    int col;

    bool operator==(const Position& p) const {
        return row == p.row && col == p.col;
    }

    bool operator!=(const Position& p) const {
        return !(*this == p);
    }
};

inline bool isInside(Position p) {
    return p.row >= 0 && p.row < 8 &&
           p.col >= 0 && p.col < 8;
}

inline Color opposite(Color c) {
    return c == Color::White ? Color::Black : Color::White;
}
