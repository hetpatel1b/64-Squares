
#pragma once

#include "ChessBoard.hpp"

class ChessEngine {
private:
    mutable ChessBoard board;
    Color turn = Color::White;

    bool isPathClear(Position from, Position to) const;

public:
    ChessEngine() = default;
    explicit ChessEngine(ChessBoard customBoard, Color startingTurn = Color::White);

    bool tryMove(Position from, Position to);

    bool isLegalMove(Position from, Position to) const;
    bool isLegalMoveForSide(Position from, Position to, Color side) const;
    bool hasLegalMoves(Color side) const;

    bool isSquareAttacked(Position square, Color byColor) const;
    bool isInCheck(Color side) const;
    bool isCheckmate(Color side) const;
    bool isCheckmate() const;
    bool isStalemate(Color side) const;
    bool isStalemate() const;
    bool isGameOver() const;
    std::optional<Color> getWinner() const;

    std::optional<Position> findKing(Color side) const;

    Color getTurn() const {
        return turn;
    }

    const ChessBoard& getBoard() const {
        return board;
    }

    void reset();
};
