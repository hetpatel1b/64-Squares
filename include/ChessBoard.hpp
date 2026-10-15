
#pragma once

#include "Piece.hpp"
#include <array>
#include <memory>

using Board = std::array<
    std::array<std::unique_ptr<Piece>, 8>, 8
>;

class ChessBoard {
private:
    Board squares{};

public:
    ChessBoard();

    void setupInitialPosition();
    void clear();

    Piece* getPiece(Position p);
    const Piece* getPiece(Position p) const;

    void movePiece(Position from, Position to);
};
