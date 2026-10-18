
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

void ChessBoard::setPiece(Position p, std::unique_ptr<Piece> piece) {
    if (!isInside(p))
        return;

    squares[p.row][p.col] = std::move(piece);
}

void ChessBoard::movePiece(Position from, Position to) {
    applyMove(from, to);
}

MoveRecord ChessBoard::applyMove(Position from, Position to) {
    MoveRecord record;
    record.from = from;
    record.to = to;

    if (isInside(to)) {
        record.capturedPiece = std::move(squares[to.row][to.col]);
    }
    if (isInside(from) && isInside(to)) {
        squares[to.row][to.col] = std::move(squares[from.row][from.col]);
    }

    return record;
}

void ChessBoard::undoMove(MoveRecord record) {
    if (isInside(record.from) && isInside(record.to)) {
        squares[record.from.row][record.from.col] = std::move(squares[record.to.row][record.to.col]);
        squares[record.to.row][record.to.col] = std::move(record.capturedPiece);
    }
}

std::optional<Position> ChessBoard::findKing(Color color) const {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* piece = squares[r][c].get();
            if (piece != nullptr &&
                piece->getColor() == color &&
                piece->getType() == PieceType::King) {
                return Position{r, c};
            }
        }
    }
    return std::nullopt;
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
