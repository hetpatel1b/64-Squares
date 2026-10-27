#include "IChessGame.hpp"
#include "EngineAdapter.hpp"
#include "Pawn.hpp"
#include "Queen.hpp"
#include "King.hpp"
#include <cassert>
#include <iostream>

void testAdapterInitialState() {
    EngineAdapter adapter;
    assert(adapter.getCurrentTurn() == Color::White);
    assert(adapter.getGameStatus() == GameStatus::Active);
    assert(!adapter.getWinner().has_value());
    assert(!adapter.getLastMove().has_value());
    assert(adapter.getMoveHistory().empty());
    assert(adapter.getCapturedPieces(Color::White).empty());
    assert(adapter.getCapturedPieces(Color::Black).empty());
    assert(adapter.getMaterialAdvantage(Color::White) == 0);
    assert(!adapter.canUndo());

    // Check piece on e2 (row 6, col 4)
    auto e2 = adapter.getPieceAt({6, 4});
    assert(e2.has_value());
    assert(e2->type == PieceType::Pawn);
    assert(e2->color == Color::White);

    // Legal destinations for e2: e3 (5, 4) and e4 (4, 4)
    auto dests = adapter.getLegalDestinations({6, 4});
    assert(dests.size() == 2);

    // Cannot query legal destinations for opponent on White's turn
    auto blackDests = adapter.getLegalDestinations({1, 4});
    assert(blackDests.empty());
}

void testMoveExecutionAndHistory() {
    EngineAdapter adapter;

    // 1. e4 (6, 4 -> 4, 4)
    bool moved1 = adapter.makeMove({6, 4}, {4, 4});
    assert(moved1);
    assert(adapter.getCurrentTurn() == Color::Black);
    assert(adapter.getLastMove().has_value());
    assert(adapter.getLastMove()->first == Position({6, 4}));
    assert(adapter.getLastMove()->second == Position({4, 4}));
    assert(adapter.getMoveHistory().size() == 1);
    assert(adapter.getMoveHistory()[0].notation == "e4");
    assert(adapter.canUndo());

    // 1... d5 (1, 3 -> 3, 3)
    bool moved2 = adapter.makeMove({1, 3}, {3, 3});
    assert(moved2);
    assert(adapter.getCurrentTurn() == Color::White);
    assert(adapter.getMoveHistory().size() == 2);
    assert(adapter.getMoveHistory()[1].notation == "d5");

    // 2. exd5 capture (4, 4 -> 3, 3)
    bool moved3 = adapter.makeMove({4, 4}, {3, 3});
    assert(moved3);
    assert(adapter.getMoveHistory().size() == 3);
    assert(adapter.getMoveHistory()[2].notation == "exd5");
    assert(adapter.getCapturedPieces(Color::White).size() == 1);
    assert(adapter.getCapturedPieces(Color::White)[0] == PieceType::Pawn);
    assert(adapter.getMaterialAdvantage(Color::White) == 1);
}

void testUndoFunctionality() {
    EngineAdapter adapter;

    // 1. e4
    adapter.makeMove({6, 4}, {4, 4});
    // 1... e5
    adapter.makeMove({1, 4}, {3, 4});

    assert(adapter.getMoveHistory().size() == 2);
    assert(adapter.getCurrentTurn() == Color::White);

    // Undo 1... e5
    bool undone1 = adapter.undo();
    assert(undone1);
    assert(adapter.getMoveHistory().size() == 1);
    assert(adapter.getCurrentTurn() == Color::Black);
    assert(adapter.getPieceAt({3, 4}) == std::nullopt);
    assert(adapter.getPieceAt({1, 4}).has_value());

    // Undo 1. e4
    bool undone2 = adapter.undo();
    assert(undone2);
    assert(adapter.getMoveHistory().empty());
    assert(adapter.getCurrentTurn() == Color::White);
    assert(!adapter.getLastMove().has_value());
    assert(!adapter.canUndo());
}

void testIllegalMovesRejected() {
    EngineAdapter adapter;

    // Moving empty square
    assert(!adapter.makeMove({4, 4}, {5, 4}));

    // Moving opponent piece
    assert(!adapter.makeMove({1, 4}, {2, 4}));

    // Moving onto own piece
    assert(!adapter.makeMove({7, 0}, {6, 0}));

    // Invalid knight jump
    assert(!adapter.makeMove({7, 1}, {4, 1}));
}

void testPromotionQueryAndExecution() {
    ChessBoard customBoard;
    customBoard.clear();
    // White pawn on 7th rank (row 1, col 0), Black King on h8 (row 0, col 7), White King on h1 (row 7, col 7)
    customBoard.setPiece({1, 0}, std::make_unique<Pawn>(Color::White));
    customBoard.setPiece({0, 7}, std::make_unique<King>(Color::Black));
    customBoard.setPiece({7, 7}, std::make_unique<King>(Color::White));

    EngineAdapter adapter(std::move(customBoard), Color::White);

    assert(adapter.isPromotionMove({1, 0}, {0, 0}));
    assert(!adapter.isPromotionMove({1, 0}, {2, 0}));

    // Make promotion move to Queen
    bool moved = adapter.makeMove({1, 0}, {0, 0}, PieceType::Queen);
    assert(moved);

    auto promoPiece = adapter.getPieceAt({0, 0});
    assert(promoPiece.has_value());
    assert(promoPiece->type == PieceType::Queen);
    assert(promoPiece->color == Color::White);
    assert(adapter.getMoveHistory().back().promotionPiece == PieceType::Queen);
}

int main() {
    std::cout << "Running Phase 4 UI & EngineAdapter Integration Tests..." << std::endl;
    testAdapterInitialState();
    testMoveExecutionAndHistory();
    testUndoFunctionality();
    testIllegalMovesRejected();
    testPromotionQueryAndExecution();
    std::cout << "All Phase 4 UI & EngineAdapter Integration Tests Passed!" << std::endl;
    return 0;
}
