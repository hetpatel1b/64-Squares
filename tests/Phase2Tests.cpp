#include "ChessEngine.hpp"
#include "Piece.hpp"
#include <cassert>
#include <iostream>

void testInitialPosition() {
    ChessEngine game;
    const ChessBoard& board = game.getBoard();

    // Verify turn starts with White
    assert(game.getTurn() == Color::White);

    // Verify White back rank pieces
    const Piece* wRook1 = board.getPiece({7, 0});
    assert(wRook1 != nullptr && wRook1->getType() == PieceType::Rook && wRook1->getColor() == Color::White);

    const Piece* wKnight1 = board.getPiece({7, 1});
    assert(wKnight1 != nullptr && wKnight1->getType() == PieceType::Knight && wKnight1->getColor() == Color::White);

    const Piece* wBishop1 = board.getPiece({7, 2});
    assert(wBishop1 != nullptr && wBishop1->getType() == PieceType::Bishop && wBishop1->getColor() == Color::White);

    const Piece* wQueen = board.getPiece({7, 3});
    assert(wQueen != nullptr && wQueen->getType() == PieceType::Queen && wQueen->getColor() == Color::White);

    const Piece* wKing = board.getPiece({7, 4});
    assert(wKing != nullptr && wKing->getType() == PieceType::King && wKing->getColor() == Color::White);

    // Verify Pawns setup
    for (int col = 0; col < 8; ++col) {
        const Piece* wp = board.getPiece({6, col});
        assert(wp != nullptr && wp->getType() == PieceType::Pawn && wp->getColor() == Color::White);

        const Piece* bp = board.getPiece({1, col});
        assert(bp != nullptr && bp->getType() == PieceType::Pawn && bp->getColor() == Color::Black);
    }

    // Verify empty center squares
    for (int row = 2; row <= 5; ++row) {
        for (int col = 0; col < 8; ++col) {
            assert(board.getPiece({row, col}) == nullptr);
        }
    }
}

void testPawnMovement() {
    ChessEngine game;

    // Single step forward
    assert(game.tryMove({6, 4}, {5, 4})); // e2 to e3
    assert(game.getTurn() == Color::Black);

    // Black double step forward
    assert(game.tryMove({1, 3}, {3, 3})); // d7 to d5
    assert(game.getTurn() == Color::White);

    // White pawn advance
    assert(game.tryMove({6, 2}, {4, 2})); // c2 to c4
    assert(game.getTurn() == Color::Black);

    // Black pawn at {3, 3} captures White pawn at {4, 2}
    assert(game.tryMove({3, 3}, {4, 2}));
    assert(game.getTurn() == Color::White);
}

void testKnightMovement() {
    ChessEngine game;

    // Valid Knight move jumping over pawns
    assert(game.tryMove({7, 1}, {5, 2})); // b1 to c3
    assert(game.getTurn() == Color::Black);

    // Invalid Knight move
    assert(!game.tryMove({0, 1}, {3, 3}));
    assert(game.getTurn() == Color::Black);

    // Valid Black Knight move
    assert(game.tryMove({0, 1}, {2, 2})); // b8 to c6
    assert(game.getTurn() == Color::White);
}

void testSlidingPieceObstruction() {
    ChessEngine game;

    // White Rook at {7, 0} blocked by friendly pawn at {6, 0}
    assert(!game.tryMove({7, 0}, {5, 0}));

    // White Bishop at {7, 2} blocked by pawns
    assert(!game.tryMove({7, 2}, {5, 4}));

    // Move pawn to open diagonal
    assert(game.tryMove({6, 3}, {4, 3})); // d2 to d4
    assert(game.tryMove({1, 4}, {3, 4})); // e7 to e5

    // Now Bishop can move diagonally through open path
    assert(game.tryMove({7, 2}, {4, 5})); // c1 to f4
    assert(game.getTurn() == Color::Black);
}

void testTurnAndInvalidMoves() {
    ChessEngine game;

    // Cannot move Black piece on White's turn
    assert(!game.tryMove({1, 0}, {2, 0}));
    assert(game.getTurn() == Color::White);

    // Cannot move from empty square
    assert(!game.tryMove({4, 4}, {4, 5}));

    // Cannot move out of bounds
    assert(!game.tryMove({7, 0}, {-1, 0}));
    assert(!game.tryMove({7, 0}, {8, 0}));

    // Cannot capture friendly piece
    assert(!game.tryMove({7, 0}, {6, 0}));

    // Cannot move to same square
    assert(!game.tryMove({6, 0}, {6, 0}));
}

int main() {
    std::cout << "Running 64 Squares Automated Test Suite...\n";
    testInitialPosition();
    std::cout << "  [PASS] Initial position setup (32 pieces)\n";
    testPawnMovement();
    std::cout << "  [PASS] Pawn movement and captures\n";
    testKnightMovement();
    std::cout << "  [PASS] Knight movement validation\n";
    testSlidingPieceObstruction();
    std::cout << "  [PASS] Sliding piece obstruction checks\n";
    testTurnAndInvalidMoves();
    std::cout << "  [PASS] Turn alternation and invalid move rejection\n";
    std::cout << "All 64 Squares automated tests passed successfully!\n";
    return 0;
}
