
#include "ChessEngine.hpp"

#include <cstdlib>

ChessEngine::ChessEngine(ChessBoard customBoard, Color startingTurn)
    : board(std::move(customBoard)), turn(startingTurn) {}

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

bool ChessEngine::isSquareAttacked(
    Position square, Color byColor
) const {
    if (!isInside(square)) {
        return false;
    }

    // 1. Pawn attacks:
    // Pawns attack diagonally forward based on their color, regardless of whether
    // the target square contains a piece or is empty.
    int pawnSourceRow = (byColor == Color::White) ? square.row + 1 : square.row - 1;
    for (int dCol : {-1, 1}) {
        Position pPos{pawnSourceRow, square.col + dCol};
        if (isInside(pPos)) {
            const Piece* piece = board.getPiece(pPos);
            if (piece != nullptr && piece->getColor() == byColor && piece->getType() == PieceType::Pawn) {
                return true;
            }
        }
    }

    // 2. Knight attacks:
    static constexpr int knightOffsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2},  {1, 2},  {2, -1},  {2, 1}
    };
    for (const auto& offset : knightOffsets) {
        Position kPos{square.row + offset[0], square.col + offset[1]};
        if (isInside(kPos)) {
            const Piece* piece = board.getPiece(kPos);
            if (piece != nullptr && piece->getColor() == byColor && piece->getType() == PieceType::Knight) {
                return true;
            }
        }
    }

    // 3. King attacks:
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            Position kPos{square.row + dr, square.col + dc};
            if (isInside(kPos)) {
                const Piece* piece = board.getPiece(kPos);
                if (piece != nullptr && piece->getColor() == byColor && piece->getType() == PieceType::King) {
                    return true;
                }
            }
        }
    }

    // 4. Diagonal sliding attacks (Bishop and Queen):
    static constexpr int diagDirs[4][2] = {
        {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
    };
    for (const auto& dir : diagDirs) {
        int r = square.row + dir[0];
        int c = square.col + dir[1];
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            const Piece* piece = board.getPiece({r, c});
            if (piece != nullptr) {
                if (piece->getColor() == byColor &&
                    (piece->getType() == PieceType::Bishop || piece->getType() == PieceType::Queen)) {
                    return true;
                }
                break; // Blocked by intervening piece
            }
            r += dir[0];
            c += dir[1];
        }
    }

    // 5. Orthogonal sliding attacks (Rook and Queen):
    static constexpr int orthoDirs[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };
    for (const auto& dir : orthoDirs) {
        int r = square.row + dir[0];
        int c = square.col + dir[1];
        while (r >= 0 && r < 8 && c >= 0 && c < 8) {
            const Piece* piece = board.getPiece({r, c});
            if (piece != nullptr) {
                if (piece->getColor() == byColor &&
                    (piece->getType() == PieceType::Rook || piece->getType() == PieceType::Queen)) {
                    return true;
                }
                break; // Blocked by intervening piece
            }
            r += dir[0];
            c += dir[1];
        }
    }

    return false;
}

std::optional<Position> ChessEngine::findKing(Color side) const {
    return board.findKing(side);
}

bool ChessEngine::isInCheck(Color side) const {
    auto kingPos = board.findKing(side);
    if (!kingPos.has_value()) {
        return false;
    }
    return isSquareAttacked(*kingPos, opposite(side));
}

bool ChessEngine::isLegalMove(Position from, Position to) const {
    return isLegalMoveForSide(from, to, turn);
}

bool ChessEngine::isLegalMoveForSide(Position from, Position to, Color side) const {
    if (!isInside(from) || !isInside(to) || from == to)
        return false;

    Piece* piece = board.getPiece(from);
    Piece* target = board.getPiece(to);

    if (piece == nullptr || piece->getColor() != side)
        return false;

    if (target != nullptr && target->getColor() == side)
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
        int dir = (side == Color::White) ? -1 : 1;
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

    // Temporarily apply candidate move to verify moving side's king safety.
    MoveRecord record = board.applyMove(from, to);
    bool kingAttacked = isInCheck(side);
    board.undoMove(std::move(record));

    return !kingAttacked;
}

bool ChessEngine::hasLegalMoves(Color side) const {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            Position from{r, c};
            const Piece* piece = board.getPiece(from);
            if (piece == nullptr || piece->getColor() != side) {
                continue;
            }

            for (int tr = 0; tr < 8; ++tr) {
                for (int tc = 0; tc < 8; ++tc) {
                    Position to{tr, tc};
                    if (isLegalMoveForSide(from, to, side)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ChessEngine::isCheckmate(Color side) const {
    return isInCheck(side) && !hasLegalMoves(side);
}

bool ChessEngine::isCheckmate() const {
    return isCheckmate(turn);
}

bool ChessEngine::isStalemate(Color side) const {
    return !isInCheck(side) && !hasLegalMoves(side);
}

bool ChessEngine::isStalemate() const {
    return isStalemate(turn);
}

bool ChessEngine::isGameOver() const {
    return isCheckmate() || isStalemate();
}

std::optional<Color> ChessEngine::getWinner() const {
    if (isCheckmate(Color::White)) {
        return Color::Black;
    }
    if (isCheckmate(Color::Black)) {
        return Color::White;
    }
    return std::nullopt;
}

bool ChessEngine::tryMove(
    Position from, Position to
) {
    if (isGameOver())
        return false;

    if (!isLegalMove(from, to))
        return false;

    // Apply legal move and switch turn.
    board.applyMove(from, to);
    turn = opposite(turn);
    return true;
}

void ChessEngine::reset() {
    board.setupInitialPosition();
    turn = Color::White;
}
