
#pragma once

#include "ChessBoard.hpp"

class ChessEngine {
private:
    ChessBoard board;
    Color turn = Color::White;

    bool isPathClear(Position from, Position to) const;

public:
    bool tryMove(Position from, Position to);

    Color getTurn() const {
        return turn;
    }

    const ChessBoard& getBoard() const {
        return board;
    }

    void reset();
};
