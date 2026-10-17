
#pragma once

#include "Piece.hpp"
#include <array>
#include <memory>
#include <optional>

using Board = std::array<
    std::array<std::unique_ptr<Piece>, 8>, 8
>;

struct MoveRecord {
    Position from{-1, -1};
    Position to{-1, -1};
    std::unique_ptr<Piece> capturedPiece{nullptr};
};

class ChessBoard {
private:
    Board squares{};

public:
    ChessBoard();

    // Board holds unique_ptr and is strictly move-only
    ChessBoard(const ChessBoard&) = delete;
    ChessBoard& operator=(const ChessBoard&) = delete;
    ChessBoard(ChessBoard&&) noexcept = default;
    ChessBoard& operator=(ChessBoard&&) noexcept = default;

    void setupInitialPosition();
    void clear();

    Piece* getPiece(Position p);
    const Piece* getPiece(Position p) const;

    void setPiece(Position p, std::unique_ptr<Piece> piece);

    void movePiece(Position from, Position to);

    MoveRecord applyMove(Position from, Position to);
    void undoMove(MoveRecord record);

    std::optional<Position> findKing(Color color) const;
};
