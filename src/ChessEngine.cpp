
#include "ChessEngine.hpp"

#include <cstdlib>

bool ChessEngine::isPathClear(
    Position from, Position to
) const {
    int dr = (to.row > from.row) -
             (to.row < from.row);

    int dc = (to.col > from.col) -
             (to.col < from.col);

    int r = from.row + dr;
    int c = from.col + dc;

    while (r != to.row || c != to.col) {
        if (board.getPiece({r, c}) != nullptr)
            return false;

        r += dr;
        c += dc;
    }

    return true;
}

bool ChessEngine::tryMove(
    Position from, Position to
) {
    if (!isInside(from) || !isInside(to))
        return false;

    if (from == to)
        return false;

    Piece* piece = board.getPiece(from);
    Piece* target = board.getPiece(to);

    if (piece == nullptr)
        return false;

    if (piece->getColor() != turn)
        return false;

    if (target != nullptr &&
        target->getColor() == turn)
        return false;

    if (!piece->canMove(from, to))
        return false;

    int dr = to.row - from.row;
    int dc = to.col - from.col;

    // Sliding pieces cannot jump over other pieces.
    PieceType type = piece->getType();

    if (type == PieceType::Rook ||
        type == PieceType::Bishop ||
        type == PieceType::Queen) {
        if (!isPathClear(from, to))
            return false;
    }

    // Pawn-specific occupancy and movement rules.
    if (type == PieceType::Pawn) {
        int dir = (turn == Color::White) ? -1 : 1;
        int rowDiff = to.row - from.row;
        int colDiff = std::abs(dc);

        // Forward movement requires an empty destination.
        if (dc == 0) {
            if (target != nullptr)
                return false;

            // A two-square move requires an empty middle square.
            if (std::abs(rowDiff) == 2) {
                Position mid{
                    from.row + dir, from.col
                };

                if (board.getPiece(mid) != nullptr)
                    return false;
            }
        } else {
            // Normal diagonal capture requires an enemy piece.
            if (colDiff != 1 || target == nullptr)
                return false;
        }
    }

    board.movePiece(from, to);
    turn = opposite(turn);

    return true;
}

void ChessEngine::reset() {
    board.setupInitialPosition();
    turn = Color::White;
}
