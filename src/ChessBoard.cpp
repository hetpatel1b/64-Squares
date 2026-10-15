
#include "ChessBoard.hpp"

#include "Pawn.hpp"
#include "Rook.hpp"
#include "Knight.hpp"
#include "Bishop.hpp"
#include "Queen.hpp"
#include "King.hpp"

ChessBoard::ChessBoard() {
    setupInitialPosition();
}

void ChessBoard::clear() {
    for (auto& row : squares) {
        for (auto& piece : row) {
            piece.reset();
        }
    }
}

Piece* ChessBoard::getPiece(Position p) {
    if (!isInside(p))
        return nullptr;

    return squares[p.row][p.col].get();
}

const Piece* ChessBoard::getPiece(Position p) const {
    if (!isInside(p))
        return nullptr;

    return squares[p.row][p.col].get();
}

void ChessBoard::movePiece(Position from, Position to) {
    squares[to.row][to.col] =
        std::move(squares[from.row][from.col]);
}

void ChessBoard::setupInitialPosition() {
    clear();

    // Black's back rank.
    squares[0][0] = std::make_unique<Rook>(Color::Black);
    squares[0][1] = std::make_unique<Knight>(Color::Black);
    squares[0][2] = std::make_unique<Bishop>(Color::Black);
    squares[0][3] = std::make_unique<Queen>(Color::Black);
    squares[0][4] = std::make_unique<King>(Color::Black);
    squares[0][5] = std::make_unique<Bishop>(Color::Black);
    squares[0][6] = std::make_unique<Knight>(Color::Black);
    squares[0][7] = std::make_unique<Rook>(Color::Black);

    // Black pawns.
    for (int c = 0; c < 8; ++c)
        squares[1][c] = std::make_unique<Pawn>(Color::Black);

    // White pawns.
    for (int c = 0; c < 8; ++c)
        squares[6][c] = std::make_unique<Pawn>(Color::White);

    // White's back rank.
    squares[7][0] = std::make_unique<Rook>(Color::White);
    squares[7][1] = std::make_unique<Knight>(Color::White);
    squares[7][2] = std::make_unique<Bishop>(Color::White);
    squares[7][3] = std::make_unique<Queen>(Color::White);
    squares[7][4] = std::make_unique<King>(Color::White);
    squares[7][5] = std::make_unique<Bishop>(Color::White);
    squares[7][6] = std::make_unique<Knight>(Color::White);
    squares[7][7] = std::make_unique<Rook>(Color::White);
}
