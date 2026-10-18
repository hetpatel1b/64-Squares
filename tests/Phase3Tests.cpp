#include "ChessEngine.hpp"
#include "Piece.hpp"
#include "Pawn.hpp"
#include "Rook.hpp"
#include "Knight.hpp"
#include "Bishop.hpp"
#include "Queen.hpp"
#include "King.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "TEST FAILURE: " << (message) << " [" #condition "] at " \
                      << __FILE__ << ":" << __LINE__ << std::endl; \
            std::exit(1); \
        } \
    } while (false)

// 1. A pawn's diagonal attack against an empty target square
void testPawnDiagonalAttacks() {
    // Test on initial board:
    ChessEngine initialGame;

    // White pawn at e2 ({6, 4}) attacks empty d3 ({5, 3}) and f3 ({5, 5})
    TEST_ASSERT(initialGame.isSquareAttacked({5, 3}, Color::White),
                "White pawn at e2 must attack empty square d3");
    TEST_ASSERT(initialGame.isSquareAttacked({5, 5}, Color::White),
                "White pawn at e2 must attack empty square f3");

    // Black pawn at d7 ({1, 3}) attacks empty c6 ({2, 2}) and e6 ({2, 4})
    TEST_ASSERT(initialGame.isSquareAttacked({2, 2}, Color::Black),
                "Black pawn at d7 must attack empty square c6");
    TEST_ASSERT(initialGame.isSquareAttacked({2, 4}, Color::Black),
                "Black pawn at d7 must attack empty square e6");

    // Test on isolated single-pawn board to ensure straight-ahead movement is NOT an attack
    ChessBoard whitePawnBoard;
    whitePawnBoard.clear();
    whitePawnBoard.setPiece({6, 4}, std::make_unique<Pawn>(Color::White)); // e2

    ChessEngine whitePawnGame(std::move(whitePawnBoard));
    TEST_ASSERT(whitePawnGame.isSquareAttacked({5, 3}, Color::White),
                "Isolated White pawn must attack diagonal d3");
    TEST_ASSERT(whitePawnGame.isSquareAttacked({5, 5}, Color::White),
                "Isolated White pawn must attack diagonal f3");
    TEST_ASSERT(!whitePawnGame.isSquareAttacked({5, 4}, Color::White),
                "Isolated White pawn must NOT attack directly forward square e3");
    TEST_ASSERT(!whitePawnGame.isSquareAttacked({4, 4}, Color::White),
                "Isolated White pawn must NOT attack two squares forward e4");
    TEST_ASSERT(!whitePawnGame.isSquareAttacked({7, 4}, Color::White),
                "Isolated White pawn must NOT attack backward");

    ChessBoard blackPawnBoard;
    blackPawnBoard.clear();
    blackPawnBoard.setPiece({1, 3}, std::make_unique<Pawn>(Color::Black)); // d7

    ChessEngine blackPawnGame(std::move(blackPawnBoard));
    TEST_ASSERT(blackPawnGame.isSquareAttacked({2, 2}, Color::Black),
                "Isolated Black pawn must attack diagonal c6");
    TEST_ASSERT(blackPawnGame.isSquareAttacked({2, 4}, Color::Black),
                "Isolated Black pawn must attack diagonal e6");
    TEST_ASSERT(!blackPawnGame.isSquareAttacked({2, 3}, Color::Black),
                "Isolated Black pawn must NOT attack directly forward square d6");
    TEST_ASSERT(!blackPawnGame.isSquareAttacked({3, 3}, Color::Black),
                "Isolated Black pawn must NOT attack two squares forward d5");
}

// 2. Knight attack detection
void testKnightAttacks() {
    // Isolated Knight test:
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({4, 4}, std::make_unique<Knight>(Color::White));

    ChessEngine customGame(std::move(customBoard));

    // 8 L-shaped target squares from {4, 4}
    static constexpr int knightOffsets[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2},  {1, 2},  {2, -1},  {2, 1}
    };
    for (const auto& off : knightOffsets) {
        Position target{4 + off[0], 4 + off[1]};
        TEST_ASSERT(customGame.isSquareAttacked(target, Color::White),
                    "Knight at {4, 4} must attack L-shaped target square");
    }

    // Non-knight squares
    TEST_ASSERT(!customGame.isSquareAttacked({4, 5}, Color::White),
                "Knight must not attack orthogonal adjacent square");
    TEST_ASSERT(!customGame.isSquareAttacked({4, 6}, Color::White),
                "Knight must not attack orthogonal square at distance 2");
    TEST_ASSERT(!customGame.isSquareAttacked({5, 5}, Color::White),
                "Knight must not attack diagonal square");

    // Initial position checks for Knight attacks
    ChessEngine initialGame;
    TEST_ASSERT(initialGame.isSquareAttacked({5, 0}, Color::White),
                "White Knight at b1 must attack a3 in initial position");
    TEST_ASSERT(initialGame.isSquareAttacked({5, 2}, Color::White),
                "White Knight at b1 must attack c3 in initial position");
    TEST_ASSERT(initialGame.isSquareAttacked({2, 5}, Color::Black),
                "Black Knight at g8 must attack f6 in initial position");
    TEST_ASSERT(initialGame.isSquareAttacked({2, 7}, Color::Black),
                "Black Knight at g8 must attack h6 in initial position");
    // Verify an unattacked square (e.g. b4 is not reached by knights or pawns)
    TEST_ASSERT(!initialGame.isSquareAttacked({4, 1}, Color::White),
                "White pieces do not attack b4 in initial position");
}

// 3. King attack detection
void testKingAttacks() {
    // Isolated King test:
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({4, 4}, std::make_unique<King>(Color::White));

    ChessEngine customGame(std::move(customBoard));

    // All 8 adjacent squares must be attacked
    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            TEST_ASSERT(customGame.isSquareAttacked({4 + dr, 4 + dc}, Color::White),
                        "King at {4, 4} must attack all adjacent squares");
        }
    }

    // Distance 2 squares must not be attacked
    TEST_ASSERT(!customGame.isSquareAttacked({4, 2}, Color::White),
                "King must not attack distance 2 square");
    TEST_ASSERT(!customGame.isSquareAttacked({2, 4}, Color::White),
                "King must not attack distance 2 square");
    TEST_ASSERT(!customGame.isSquareAttacked({6, 6}, Color::White),
                "King must not attack distance 2 diagonal square");
}

// 4. Sliding-piece attack detection with clear path vs blocked by intervening piece
void testSlidingPieceAttacksClearAndBlocked() {
    // Test clear vs blocked attacks on an isolated board
    ChessBoard customBoard;
    customBoard.clear();
    // Rook at {7, 0}, Pawn blocking at {6, 0}
    customBoard.setPiece({7, 0}, std::make_unique<Rook>(Color::White));
    customBoard.setPiece({6, 0}, std::make_unique<Pawn>(Color::White));

    // Bishop at {7, 7} with clear diagonal
    customBoard.setPiece({7, 7}, std::make_unique<Bishop>(Color::White));

    ChessEngine game(std::move(customBoard));

    // Rook at {7, 0} attacks {6, 0} where the blocking pawn is
    TEST_ASSERT(game.isSquareAttacked({6, 0}, Color::White),
                "Rook attacks the adjacent square occupied by pawn");

    // But Rook attack beyond {6, 0} along the file is BLOCKED
    TEST_ASSERT(!game.isSquareAttacked({5, 0}, Color::White),
                "Rook attack on {5, 0} must be blocked by {6, 0} pawn");
    TEST_ASSERT(!game.isSquareAttacked({4, 0}, Color::White),
                "Rook attack on {4, 0} must be blocked by {6, 0} pawn");
    TEST_ASSERT(!game.isSquareAttacked({1, 0}, Color::White),
                "Rook attack on {1, 0} must be blocked by {6, 0} pawn");

    // Bishop at {7, 7} has clear diagonal to {6, 6}, {5, 5}, {4, 4}, etc.
    TEST_ASSERT(game.isSquareAttacked({6, 6}, Color::White), "Bishop attacks {6, 6}");
    TEST_ASSERT(game.isSquareAttacked({5, 5}, Color::White), "Bishop attacks {5, 5}");
    TEST_ASSERT(game.isSquareAttacked({4, 4}, Color::White), "Bishop attacks {4, 4}");
    TEST_ASSERT(game.isSquareAttacked({0, 0}, Color::White), "Bishop attacks {0, 0}");

    // Also test in full game flow:
    ChessEngine fullGame;
    // Open diagonal path with 1. e4 d5
    TEST_ASSERT(fullGame.tryMove({6, 4}, {4, 4}), "1. e4");
    TEST_ASSERT(fullGame.tryMove({1, 3}, {3, 3}), "1... d5");

    // Bishop at f1 ({7, 5}) now has a clear diagonal to b5 ({3, 1}) and a6 ({2, 0})
    TEST_ASSERT(fullGame.isSquareAttacked({4, 2}, Color::White),
                "White Bishop at f1 must attack c4 with open diagonal");
    TEST_ASSERT(fullGame.isSquareAttacked({3, 1}, Color::White),
                "White Bishop at f1 must attack b5 with open diagonal");
    TEST_ASSERT(fullGame.isSquareAttacked({2, 0}, Color::White),
                "White Bishop at f1 must attack a6 with open diagonal");

    // Queen at d1 ({7, 3}) has clear diagonal to h5 ({3, 7})
    TEST_ASSERT(fullGame.isSquareAttacked({3, 7}, Color::White),
                "White Queen at d1 must attack h5 with open diagonal");
}

// 5. A legal move that gives check and resolving an existing check
void testGiveCheckAndResolveCheck() {
    ChessEngine game;

    // 1. e4 d5
    TEST_ASSERT(game.tryMove({6, 4}, {4, 4}), "1. e4");
    TEST_ASSERT(game.tryMove({1, 3}, {3, 3}), "1... d5");

    TEST_ASSERT(!game.isInCheck(Color::Black), "Black must not be in check before Bb5+");
    TEST_ASSERT(!game.isInCheck(Color::White), "White must not be in check");

    // 2. Bb5+ ({7, 5} -> {3, 1}) gives check to Black King at e8 ({0, 4})
    TEST_ASSERT(game.tryMove({7, 5}, {3, 1}), "2. Bb5+ must be a legal move");
    TEST_ASSERT(game.isInCheck(Color::Black), "Black King must be in check after Bb5+");
    TEST_ASSERT(!game.isInCheck(Color::White), "White King must not be in check");
    TEST_ASSERT(game.getTurn() == Color::Black, "Turn must switch to Black");

    // Attempt an illegal move for Black that fails to resolve check: 2... a6 ({1, 0} -> {2, 0})
    TEST_ASSERT(!game.tryMove({1, 0}, {2, 0}),
                "Move that leaves Black King in check must be rejected");
    TEST_ASSERT(game.getTurn() == Color::Black, "Turn must remain Black after rejected move");
    TEST_ASSERT(game.isInCheck(Color::Black), "Black must still be in check after rejected move");
    TEST_ASSERT(game.getBoard().getPiece({1, 0}) != nullptr, "a7 pawn must not have moved");
    TEST_ASSERT(game.getBoard().getPiece({2, 0}) == nullptr, "a6 square must remain empty");

    // Legal move resolving check: 2... c6 ({1, 2} -> {2, 2}) blocks the bishop's line of sight
    TEST_ASSERT(game.tryMove({1, 2}, {2, 2}), "2... c6 blocking check must be legal");
    TEST_ASSERT(!game.isInCheck(Color::Black), "Black must no longer be in check after blocking");
    TEST_ASSERT(game.getTurn() == Color::White, "Turn must switch to White");
}

// 6. Rejection of a move that exposes the moving side's king (Pin)
void testPinPreventsExposingKing() {
    // Setup:
    // Black King at {0, 4} (e8)
    // Black Knight at {1, 4} (e7) — pinned to king along e-file
    // Black Rook at {2, 4} (e6)
    // White Queen at {6, 4} (e2) — pinner
    // White King at {7, 4} (e1)
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({0, 4}, std::make_unique<King>(Color::Black));
    customBoard.setPiece({1, 4}, std::make_unique<Knight>(Color::Black));
    customBoard.setPiece({6, 4}, std::make_unique<Queen>(Color::White));
    customBoard.setPiece({7, 4}, std::make_unique<King>(Color::White));

    ChessEngine game(std::move(customBoard), Color::Black);

    // Black Knight at e7 ({1, 4}) is pinned to Black King at e8 ({0, 4}) by White Queen at e2 ({6, 4})
    // Moving the pinned knight to g6 ({2, 6}) or f5 ({3, 5}) exposes the King to check
    TEST_ASSERT(!game.tryMove({1, 4}, {2, 6}),
                "Moving pinned Knight off e-file must be rejected");
    TEST_ASSERT(!game.tryMove({1, 4}, {3, 5}),
                "Moving pinned Knight to f5 must be rejected");
    TEST_ASSERT(!game.tryMove({1, 4}, {0, 2}),
                "Moving pinned Knight to c8 must be rejected");

    TEST_ASSERT(game.getTurn() == Color::Black, "Turn must remain Black");
    const Piece* pinnedKnight = game.getBoard().getPiece({1, 4});
    TEST_ASSERT(pinnedKnight != nullptr && pinnedKnight->getType() == PieceType::Knight,
                "Pinned Knight must still be at e7");
    TEST_ASSERT(game.getBoard().getPiece({2, 6}) == nullptr, "g6 square must remain empty");

    // Pinned Rook: can move along the pin line, but cannot move off it
    ChessBoard rookPinBoard;
    rookPinBoard.clear();
    rookPinBoard.setPiece({0, 4}, std::make_unique<King>(Color::Black));
    rookPinBoard.setPiece({2, 4}, std::make_unique<Rook>(Color::Black));
    rookPinBoard.setPiece({6, 4}, std::make_unique<Queen>(Color::White));
    rookPinBoard.setPiece({7, 4}, std::make_unique<King>(Color::White));

    ChessEngine rookGame(std::move(rookPinBoard), Color::Black);

    // Moving pinned rook off the e-file (horizontally) exposes the king -> MUST BE REJECTED
    TEST_ASSERT(!rookGame.tryMove({2, 4}, {2, 3}), "Pinned Rook moving off pin file must be rejected");
    TEST_ASSERT(!rookGame.tryMove({2, 4}, {2, 5}), "Pinned Rook moving off pin file must be rejected");
    TEST_ASSERT(rookGame.getTurn() == Color::Black, "Turn must remain Black");

    // Moving pinned rook along the e-file maintains the shield -> MUST BE ALLOWED
    TEST_ASSERT(rookGame.tryMove({2, 4}, {3, 4}), "Pinned Rook moving along pin file is legal");
    TEST_ASSERT(rookGame.getTurn() == Color::White, "Turn switches to White after legal move along pin line");
}

// 7. Rejection of a king move onto an attacked square
void testKingCannotMoveIntoCheck() {
    // Construct a clean, isolated position:
    // White King at {4, 4}, Black King at {0, 0}, Black Rook at {0, 5}
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({4, 4}, std::make_unique<King>(Color::White));
    customBoard.setPiece({0, 0}, std::make_unique<King>(Color::Black));
    customBoard.setPiece({0, 5}, std::make_unique<Rook>(Color::Black));

    ChessEngine game(std::move(customBoard), Color::White);

    // Verify Black Rook attacks all squares in column 5: {3, 5}, {4, 5}, {5, 5}
    TEST_ASSERT(game.isSquareAttacked({4, 5}, Color::Black), "Square {4, 5} is attacked by Black Rook");
    TEST_ASSERT(game.isSquareAttacked({3, 5}, Color::Black), "Square {3, 5} is attacked by Black Rook");
    TEST_ASSERT(game.isSquareAttacked({5, 5}, Color::Black), "Square {5, 5} is attacked by Black Rook");

    // King cannot move onto attacked squares
    TEST_ASSERT(!game.tryMove({4, 4}, {4, 5}), "White King cannot move onto attacked square {4, 5}");
    TEST_ASSERT(!game.tryMove({4, 4}, {3, 5}), "White King cannot move onto attacked square {3, 5}");
    TEST_ASSERT(!game.tryMove({4, 4}, {5, 5}), "White King cannot move onto attacked square {5, 5}");

    // King CAN move to safe square {4, 3}
    TEST_ASSERT(game.tryMove({4, 4}, {4, 3}), "White King can move to safe square {4, 3}");
    TEST_ASSERT(game.getTurn() == Color::Black, "Turn switches to Black after legal king move");
}

// 8. King cannot move adjacent to enemy king
void testKingProximity() {
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({4, 4}, std::make_unique<King>(Color::White));
    customBoard.setPiece({4, 6}, std::make_unique<King>(Color::Black));

    ChessEngine game(std::move(customBoard), Color::White);

    // Square {4, 5} is between the two kings (adjacent to Black King)
    TEST_ASSERT(game.isSquareAttacked({4, 5}, Color::Black),
                "Square {4, 5} is adjacent to and attacked by Black King");

    // White King cannot move to {4, 5}
    TEST_ASSERT(!game.tryMove({4, 4}, {4, 5}),
                "White King cannot move adjacent to Black King");
    TEST_ASSERT(game.getTurn() == Color::White, "Turn remains White");
}

// 9. Simulated capture can be undone without losing the captured piece
void testSimulatedCaptureUndoIntegrity() {
    // Setup:
    // White King at {7, 4} (e1)
    // White Knight at {6, 4} (e2)
    // Black Queen at {0, 4} (e8)
    // Black Bishop at {5, 2} (c3)
    // Black King at {0, 0} (a8)
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({7, 4}, std::make_unique<King>(Color::White));
    customBoard.setPiece({6, 4}, std::make_unique<Knight>(Color::White));
    customBoard.setPiece({0, 4}, std::make_unique<Queen>(Color::Black));
    customBoard.setPiece({5, 2}, std::make_unique<Bishop>(Color::Black));
    customBoard.setPiece({0, 0}, std::make_unique<King>(Color::Black));

    ChessEngine game(std::move(customBoard), Color::White);

    // White Knight at {6, 4} attempts to capture Black Bishop at {5, 2}.
    // Knight move geometry is valid (dr=-1, dc=-2) and destination has an enemy piece.
    // However, moving the knight opens the e-file and exposes the White King to the Black Queen!
    TEST_ASSERT(!game.tryMove({6, 4}, {5, 2}),
                "Capturing move that exposes king to check must be rejected");

    // Board state after rollback must be completely intact:
    TEST_ASSERT(game.getTurn() == Color::White, "Turn must remain White");

    const Piece* knight = game.getBoard().getPiece({6, 4});
    TEST_ASSERT(knight != nullptr, "White Knight must be present at e2");
    TEST_ASSERT(knight->getType() == PieceType::Knight, "Piece at e2 must be Knight");
    TEST_ASSERT(knight->getColor() == Color::White, "Knight at e2 must be White");

    const Piece* bishop = game.getBoard().getPiece({5, 2});
    TEST_ASSERT(bishop != nullptr, "Captured Black Bishop at c3 must NOT be lost");
    TEST_ASSERT(bishop->getType() == PieceType::Bishop, "Piece at c3 must remain Bishop");
    TEST_ASSERT(bishop->getColor() == Color::Black, "Piece at c3 must remain Black");

    // Verify low-level applyMove and undoMove directly
    ChessBoard testBoard;
    testBoard.clear();
    testBoard.setPiece({1, 1}, std::make_unique<Rook>(Color::White));
    testBoard.setPiece({1, 5}, std::make_unique<Bishop>(Color::Black));

    MoveRecord record = testBoard.applyMove({1, 1}, {1, 5});
    TEST_ASSERT(testBoard.getPiece({1, 1}) == nullptr, "Source square must be empty after applyMove");
    TEST_ASSERT(testBoard.getPiece({1, 5}) != nullptr &&
                testBoard.getPiece({1, 5})->getType() == PieceType::Rook,
                "Destination must hold moved rook");
    TEST_ASSERT(record.capturedPiece != nullptr &&
                record.capturedPiece->getType() == PieceType::Bishop &&
                record.capturedPiece->getColor() == Color::Black,
                "MoveRecord must preserve captured piece dynamic type and color");

    testBoard.undoMove(std::move(record));
    TEST_ASSERT(testBoard.getPiece({1, 1}) != nullptr &&
                testBoard.getPiece({1, 1})->getType() == PieceType::Rook,
                "Rook must be back at source square after undoMove");
    TEST_ASSERT(testBoard.getPiece({1, 5}) != nullptr &&
                testBoard.getPiece({1, 5})->getType() == PieceType::Bishop &&
                testBoard.getPiece({1, 5})->getColor() == Color::Black,
                "Captured Bishop must be completely restored after undoMove");
}

// 10. A failed move leaves the board and turn unchanged
void testFailedMoveLeavesBoardUnchanged() {
    ChessEngine game;
    Color turnBefore = game.getTurn();

    // Snapshot board piece types and colors
    struct SquareSnapshot {
        bool hasPiece;
        PieceType type;
        Color color;
    };
    SquareSnapshot before[8][8];
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = game.getBoard().getPiece({r, c});
            if (p != nullptr) {
                before[r][c] = {true, p->getType(), p->getColor()};
            } else {
                before[r][c] = {false, PieceType::Pawn, Color::White};
            }
        }
    }

    // Attempt illegal moves
    TEST_ASSERT(!game.tryMove({7, 0}, {5, 0}), "Illegal move blocked rook");
    TEST_ASSERT(!game.tryMove({6, 4}, {3, 4}), "Illegal pawn 3-step");
    TEST_ASSERT(!game.tryMove({0, 0}, {1, 0}), "Wrong turn piece");

    TEST_ASSERT(game.getTurn() == turnBefore, "Turn must remain identical");

    // Verify all 64 squares match snapshot exactly
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* p = game.getBoard().getPiece({r, c});
            if (before[r][c].hasPiece) {
                TEST_ASSERT(p != nullptr, "Square must still contain a piece");
                TEST_ASSERT(p->getType() == before[r][c].type, "Piece type must match");
                TEST_ASSERT(p->getColor() == before[r][c].color, "Piece color must match");
            } else {
                TEST_ASSERT(p == nullptr, "Empty square must remain empty");
            }
        }
    }
}

// 11. Pinned piece still attacks squares for king safety
void testPinnedPieceStillAttacks() {
    ChessBoard customBoard;
    customBoard.clear();
    // White King at {7, 4}, White Bishop at {6, 4}, Black Queen at {0, 4}
    // Bishop at {6, 4} is pinned along the e-file
    customBoard.setPiece({7, 4}, std::make_unique<King>(Color::White));
    customBoard.setPiece({6, 4}, std::make_unique<Bishop>(Color::White));
    customBoard.setPiece({0, 4}, std::make_unique<Queen>(Color::Black));
    customBoard.setPiece({0, 0}, std::make_unique<King>(Color::Black));

    ChessEngine game(std::move(customBoard), Color::White);

    // The pinned bishop still attacks its diagonal squares
    TEST_ASSERT(game.isSquareAttacked({5, 3}, Color::White),
                "Pinned Bishop must still attack diagonal square d3");
    TEST_ASSERT(game.isSquareAttacked({5, 5}, Color::White),
                "Pinned Bishop must still attack diagonal square f3");
}

// 12. Missing king safety and robust handling of invalid positions
void testMissingKingSafety() {
    ChessBoard emptyBoard;
    emptyBoard.clear();

    ChessEngine game(std::move(emptyBoard), Color::White);

    // Board with no kings must not crash or dereference null
    TEST_ASSERT(!game.findKing(Color::White).has_value(), "No white king on empty board");
    TEST_ASSERT(!game.findKing(Color::Black).has_value(), "No black king on empty board");
    TEST_ASSERT(!game.isInCheck(Color::White), "Missing king safely evaluates to not in check");
    TEST_ASSERT(!game.isInCheck(Color::Black), "Missing king safely evaluates to not in check");

    // Out of bounds attack queries safely return false
    TEST_ASSERT(!game.isSquareAttacked({-1, -1}, Color::White), "Out of bounds is not attacked");
    TEST_ASSERT(!game.isSquareAttacked({8, 8}, Color::Black), "Out of bounds is not attacked");
    TEST_ASSERT(!game.isSquareAttacked({100, 100}, Color::White), "Out of bounds is not attacked");
}

// 13. Ordinary Phase 2 moves and captures continue to work
void testPhase2MoveContinuity() {
    ChessEngine game;

    // Pawn 1-step
    TEST_ASSERT(game.tryMove({6, 4}, {5, 4}), "e2 to e3");
    TEST_ASSERT(game.getTurn() == Color::Black, "Turn Black");

    // Pawn 2-step
    TEST_ASSERT(game.tryMove({1, 3}, {3, 3}), "d7 to d5");
    TEST_ASSERT(game.getTurn() == Color::White, "Turn White");

    // Pawn 2-step
    TEST_ASSERT(game.tryMove({6, 2}, {4, 2}), "c2 to c4");
    TEST_ASSERT(game.getTurn() == Color::Black, "Turn Black");

    // Pawn capture
    TEST_ASSERT(game.tryMove({3, 3}, {4, 2}), "d5 captures c4");
    TEST_ASSERT(game.getTurn() == Color::White, "Turn White");
}

// 14. Checkmate detection and winner identification (Fool's Mate)
void testFoolsMateCheckmateAndWinner() {
    ChessEngine game;

    // 1. f3 e5
    TEST_ASSERT(game.tryMove({6, 5}, {5, 5}), "1. f3");
    TEST_ASSERT(game.tryMove({1, 4}, {3, 4}), "1... e5");

    // 2. g4 Qh4#
    TEST_ASSERT(game.tryMove({6, 6}, {4, 6}), "2. g4");
    TEST_ASSERT(!game.isCheckmate(), "Game is not checkmate before 2... Qh4");
    TEST_ASSERT(game.tryMove({0, 3}, {4, 7}), "2... Qh4#");

    // White is now checkmated by Black
    TEST_ASSERT(game.isInCheck(Color::White), "White must be in check");
    TEST_ASSERT(!game.hasLegalMoves(Color::White), "White must have no legal moves");
    TEST_ASSERT(game.isCheckmate(Color::White), "White must be checkmated");
    TEST_ASSERT(game.isCheckmate(), "Current side (White) is checkmated");
    TEST_ASSERT(!game.isStalemate(), "Game is not stalemate");
    TEST_ASSERT(game.isGameOver(), "Game must be over on checkmate");

    auto winner = game.getWinner();
    TEST_ASSERT(winner.has_value(), "Winner must be present on checkmate");
    TEST_ASSERT(*winner == Color::Black, "Black must be the winner in Fool's mate");

    // Any move attempted after checkmate must be rejected
    TEST_ASSERT(!game.tryMove({7, 4}, {6, 4}), "Move attempted after checkmate must be rejected");
}

// 15. Checkmate detection and winner identification (Scholar's Mate)
void testScholarsMateCheckmateAndWinner() {
    ChessEngine game;

    // 1. e4 e5
    TEST_ASSERT(game.tryMove({6, 4}, {4, 4}), "1. e4");
    TEST_ASSERT(game.tryMove({1, 4}, {3, 4}), "1... e5");

    // 2. Bc4 Nc6
    TEST_ASSERT(game.tryMove({7, 5}, {4, 2}), "2. Bc4");
    TEST_ASSERT(game.tryMove({0, 1}, {2, 2}), "2... Nc6");

    // 3. Qh5 a6
    TEST_ASSERT(game.tryMove({7, 3}, {3, 7}), "3. Qh5");
    TEST_ASSERT(game.tryMove({1, 0}, {2, 0}), "3... a6");

    // 4. Qxf7#
    TEST_ASSERT(game.tryMove({3, 7}, {1, 5}), "4. Qxf7#");

    // Black is now checkmated by White
    TEST_ASSERT(game.isInCheck(Color::Black), "Black must be in check");
    TEST_ASSERT(!game.hasLegalMoves(Color::Black), "Black must have no legal moves");
    TEST_ASSERT(game.isCheckmate(Color::Black), "Black must be checkmated");
    TEST_ASSERT(game.isCheckmate(), "Current side (Black) is checkmated");
    TEST_ASSERT(game.isGameOver(), "Game must be over on checkmate");

    auto winner = game.getWinner();
    TEST_ASSERT(winner.has_value(), "Winner must be present on checkmate");
    TEST_ASSERT(*winner == Color::White, "White must be the winner in Scholar's mate");
}

// 16. Stalemate detection (no legal moves, not in check -> Draw)
void testStalemateDetection() {
    // Classic stalemate position:
    // Black King at a8 {0, 0}
    // White King at a6 {2, 0}
    // White Queen at c7 {1, 2}
    ChessBoard customBoard;
    customBoard.clear();
    customBoard.setPiece({0, 0}, std::make_unique<King>(Color::Black));
    customBoard.setPiece({2, 0}, std::make_unique<King>(Color::White));
    customBoard.setPiece({1, 2}, std::make_unique<Queen>(Color::White));

    ChessEngine game(std::move(customBoard), Color::Black);

    TEST_ASSERT(!game.isInCheck(Color::Black), "Black King must NOT be in check");
    TEST_ASSERT(!game.hasLegalMoves(Color::Black), "Black must have no legal moves");
    TEST_ASSERT(game.isStalemate(Color::Black), "Black must be in stalemate");
    TEST_ASSERT(game.isStalemate(), "Current side is in stalemate");
    TEST_ASSERT(!game.isCheckmate(Color::Black), "Stalemate is not checkmate");
    TEST_ASSERT(game.isGameOver(), "Game must be over on stalemate");
    TEST_ASSERT(!game.getWinner().has_value(), "Stalemate produces no winner (draw)");
}

int main() {
    std::cout << "Running 64 Squares Phase 3 Automated Tests...\n";

    testPawnDiagonalAttacks();
    std::cout << "  [PASS] 1. Pawn diagonal attack detection on empty squares\n";

    testKnightAttacks();
    std::cout << "  [PASS] 2. Knight attack detection\n";

    testKingAttacks();
    std::cout << "  [PASS] 3. King attack detection\n";

    testSlidingPieceAttacksClearAndBlocked();
    std::cout << "  [PASS] 4. Sliding-piece attacks with clear and blocked paths\n";

    testGiveCheckAndResolveCheck();
    std::cout << "  [PASS] 5. Legal moves giving check and resolving check\n";

    testPinPreventsExposingKing();
    std::cout << "  [PASS] 6. Move rejection when piece is pinned to King\n";

    testKingCannotMoveIntoCheck();
    std::cout << "  [PASS] 7. Rejection of king move onto attacked square\n";

    testKingProximity();
    std::cout << "  [PASS] 8. King cannot move adjacent to enemy king\n";

    testSimulatedCaptureUndoIntegrity();
    std::cout << "  [PASS] 9. Reversible move undo integrity on simulated captures\n";

    testFailedMoveLeavesBoardUnchanged();
    std::cout << "  [PASS] 10. Failed move leaves board state and turn unchanged\n";

    testPinnedPieceStillAttacks();
    std::cout << "  [PASS] 11. Pinned pieces still attack squares for king safety\n";

    testMissingKingSafety();
    std::cout << "  [PASS] 12. Safe handling of missing kings and boundary positions\n";

    testPhase2MoveContinuity();
    std::cout << "  [PASS] 13. Backwards compatibility with Phase 2 moves and captures\n";

    testFoolsMateCheckmateAndWinner();
    std::cout << "  [PASS] 14. Checkmate and winner detection (Fool's Mate -> Black wins)\n";

    testScholarsMateCheckmateAndWinner();
    std::cout << "  [PASS] 15. Checkmate and winner detection (Scholar's Mate -> White wins)\n";

    testStalemateDetection();
    std::cout << "  [PASS] 16. Stalemate detection (Draw with no winner)\n";

    std::cout << "All automated tests passed successfully!\n";
    return 0;
}
