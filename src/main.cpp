
#include "ChessEngine.hpp"
#include <iostream>

int main() {
    ChessEngine game;

    const ChessBoard& board = game.getBoard();

    std::cout << "64 Squares Phase 2\n";

    const Piece* p = board.getPiece({7, 1});

    if (p != nullptr &&
        p->getType() == PieceType::Knight) {
        std::cout << "Initial position: OK\n";
    }

    bool ok = game.tryMove({7, 1}, {5, 2});

    std::cout << "Knight move: "
              << (ok ? "Accepted" : "Rejected")
              << '\n';

    std::cout << "Black to move: "
              << (game.getTurn() == Color::Black)
              << '\n';

    return 0;
}
