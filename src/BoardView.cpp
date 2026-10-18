#include "BoardView.hpp"
#include <string>

BoardView::BoardView(int width, int height)
    : renderer(width, height),
      statusMessage("White to move") {}

void BoardView::init() {
    renderer.init("64 Squares");
}

void BoardView::cleanup() {
    renderer.close();
}

bool BoardView::shouldClose() const {
    return renderer.shouldClose();
}

std::string BoardView::formatSquare(Position pos) const {
    if (!isInside(pos)) return "??";
    char file = static_cast<char>('a' + pos.col);
    char rank = static_cast<char>('8' - pos.row);
    return std::string{ file, rank };
}

std::string BoardView::formatPieceName(const Piece* piece) const {
    if (piece == nullptr) return "Piece";
    std::string name = (piece->getColor() == Color::White) ? "White " : "Black ";
    switch (piece->getType()) {
        case PieceType::Pawn:   name += "Pawn"; break;
        case PieceType::Rook:   name += "Rook"; break;
        case PieceType::Knight: name += "Knight"; break;
        case PieceType::Bishop: name += "Bishop"; break;
        case PieceType::Queen:  name += "Queen"; break;
        case PieceType::King:   name += "King"; break;
    }
    return name;
}

void BoardView::processInput(ChessEngine& engine) {
    // Reset board
    if (renderer.isKeyRPressed()) {
        engine.reset();
        selectedPos.reset();
        statusMessage = "Board reset to initial position";
        return;
    }

    // Deselect
    if (renderer.isKeyEscapePressed() || renderer.isRightMouseButtonPressed()) {
        if (selectedPos.has_value()) {
            selectedPos.reset();
            statusMessage = "Selection cleared";
        }
        return;
    }

    // Left mouse click handling
    if (renderer.isLeftMouseButtonPressed()) {
        int row = -1, col = -1;
        bool onBoard = renderer.getMouseBoardSquare(row, col);

        if (!onBoard) {
            selectedPos.reset();
            return;
        }

        Position clickedPos{ row, col };
        const ChessBoard& board = engine.getBoard();
        const Piece* clickedPiece = board.getPiece(clickedPos);
        Color turn = engine.getTurn();

        // If game is already over, prevent further moves
        if (engine.isGameOver()) {
            if (engine.isCheckmate()) {
                std::string winner = (engine.getWinner() == Color::White) ? "White" : "Black";
                statusMessage = "Game over — Checkmate! " + winner + " wins! Press [R] to restart.";
            } else {
                statusMessage = "Game over — Stalemate! Press [R] to restart.";
            }
            selectedPos.reset();
            return;
        }

        if (!selectedPos.has_value()) {
            if (clickedPiece != nullptr) {
                if (clickedPiece->getColor() == turn) {
                    selectedPos = clickedPos;
                    statusMessage = "Selected " + formatPieceName(clickedPiece) + " at " + formatSquare(clickedPos);
                } else {
                    statusMessage = (turn == Color::White ? "White's turn" : "Black's turn");
                }
            }
        } else {
            Position from = *selectedPos;
            if (from == clickedPos) {
                selectedPos.reset();
                statusMessage = "Deselected";
            } else if (clickedPiece != nullptr && clickedPiece->getColor() == turn) {
                selectedPos = clickedPos;
                statusMessage = "Selected " + formatPieceName(clickedPiece) + " at " + formatSquare(clickedPos);
            } else {
                const Piece* movingPiece = board.getPiece(from);
                std::string pieceDesc = formatPieceName(movingPiece);
                bool moved = engine.tryMove(from, clickedPos);

                if (moved) {
                    if (engine.isCheckmate()) {
                        std::string winner = (engine.getWinner() == Color::White) ? "White" : "Black";
                        statusMessage = "Checkmate! " + winner + " wins!";
                    } else if (engine.isStalemate()) {
                        statusMessage = "Stalemate! Game is a draw.";
                    } else if (engine.isInCheck(engine.getTurn())) {
                        std::string side = (engine.getTurn() == Color::White) ? "White" : "Black";
                        statusMessage = pieceDesc + " moved: " + formatSquare(from) + " -> " + formatSquare(clickedPos) + " (Check!)";
                    } else {
                        statusMessage = pieceDesc + " moved: " + formatSquare(from) + " -> " + formatSquare(clickedPos);
                    }
                    selectedPos.reset();
                } else {
                    statusMessage = "Illegal move rejected for " + pieceDesc;
                }
            }
        }
    }
}

void BoardView::render(const ChessEngine& engine) {
    int hoverRow = -1, hoverCol = -1;
    renderer.getMouseBoardSquare(hoverRow, hoverCol);

    int selRow = selectedPos.has_value() ? selectedPos->row : -1;
    int selCol = selectedPos.has_value() ? selectedPos->col : -1;

    // Highlight king square when in check
    int checkRow = -1, checkCol = -1;
    if (engine.isInCheck(engine.getTurn())) {
        auto kingPos = engine.findKing(engine.getTurn());
        if (kingPos.has_value()) {
            checkRow = kingPos->row;
            checkCol = kingPos->col;
        }
    }

    renderer.beginDrawing();

    // 1. Draw chessboard grid, highlights, and coordinates
    renderer.drawBoard(selRow, selCol, hoverRow, hoverCol, checkRow, checkCol);

    // 2. Draw all pieces currently on board
    const ChessBoard& board = engine.getBoard();
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece* piece = board.getPiece({ r, c });
            if (piece != nullptr) {
                char typeChar = 'P';
                switch (piece->getType()) {
                    case PieceType::Pawn:   typeChar = 'P'; break;
                    case PieceType::Rook:   typeChar = 'R'; break;
                    case PieceType::Knight: typeChar = 'N'; break;
                    case PieceType::Bishop: typeChar = 'B'; break;
                    case PieceType::Queen:  typeChar = 'Q'; break;
                    case PieceType::King:   typeChar = 'K'; break;
                }
                bool isWhite = (piece->getColor() == Color::White);
                renderer.drawPiece(r, c, typeChar, isWhite);
            }
        }
    }

    // 3. Draw header, turn indicator, and status UI
    std::string turnText;
    if (engine.isCheckmate()) {
        std::string winner = (engine.getWinner() == Color::White) ? "White" : "Black";
        turnText = "Checkmate! " + winner + " wins!";
    } else if (engine.isStalemate()) {
        turnText = "Stalemate! Game Draw";
    } else if (engine.isInCheck(engine.getTurn())) {
        turnText = (engine.getTurn() == Color::White) ? "White in check!" : "Black in check!";
    } else {
        turnText = (engine.getTurn() == Color::White) ? "White to move" : "Black to move";
    }

    renderer.drawUI(turnText, engine.getTurn() == Color::White, statusMessage);

    renderer.endDrawing();
}
