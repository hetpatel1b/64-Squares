#include "BoardView.hpp"
#include "EngineAdapter.hpp"
#include <algorithm>

BoardView::BoardView(int width, int height)
    : game(std::make_shared<EngineAdapter>()),
      renderer(width, height) {}

BoardView::BoardView(std::shared_ptr<IChessGame> engineGame, int width, int height)
    : game(std::move(engineGame)),
      renderer(width, height) {}

void BoardView::init() {
    renderer.init("64 Squares — Desktop Chess");
}

void BoardView::cleanup() {
    renderer.close();
}

bool BoardView::shouldClose() const {
    return renderer.shouldClose();
}

void BoardView::setEngine(std::shared_ptr<IChessGame> engineGame) {
    game = std::move(engineGame);
    selectedPos.reset();
    legalDestinations.clear();
    pendingPromotion.reset();
}

void BoardView::showToast(const std::string& message, float duration) {
    toastMessage = message;
    toastTimer = duration;
}

std::string BoardView::formatSquare(Position pos) const {
    if (!isInside(pos)) return "??";
    char file = static_cast<char>('a' + pos.col);
    char rank = static_cast<char>('8' - pos.row);
    return std::string{ file, rank };
}

std::string BoardView::formatPieceName(PieceType type, Color color) const {
    std::string name = (color == Color::White) ? "White " : "Black ";
    switch (type) {
        case PieceType::Pawn:   name += "Pawn"; break;
        case PieceType::Rook:   name += "Rook"; break;
        case PieceType::Knight: name += "Knight"; break;
        case PieceType::Bishop: name += "Bishop"; break;
        case PieceType::Queen:  name += "Queen"; break;
        case PieceType::King:   name += "King"; break;
    }
    return name;
}

void BoardView::processInput() {
    renderer.updateLayout();

    if (toastTimer > 0.0f) {
        toastTimer -= 0.0166f;
    }

    // Modal dialogs capture input and suspend board clicks
    if (pendingPromotion.has_value() || showSettingsModal || showConfirmModal) {
        if (renderer.isKeyEscapePressed()) {
            if (pendingPromotion.has_value()) {
                pendingPromotion.reset();
                selectedPos.reset();
                legalDestinations.clear();
                showToast("Promotion cancelled");
            } else if (showSettingsModal) {
                showSettingsModal = false;
            } else if (showConfirmModal) {
                showConfirmModal = false;
            }
        }
        return;
    }

    // Keyboard shortcuts
    if (renderer.isKeyEscapePressed() || renderer.isRightMouseButtonPressed()) {
        if (selectedPos.has_value()) {
            selectedPos.reset();
            legalDestinations.clear();
            showToast("Selection cleared");
        }
        return;
    }

    if (renderer.isKeyFPressed()) {
        settings.flipped = !settings.flipped;
        showToast(settings.flipped ? "Perspective: Black" : "Perspective: White");
        return;
    }

    if (renderer.isKeyUPressed()) {
        if (game->canUndo()) {
            game->undo();
            selectedPos.reset();
            legalDestinations.clear();
            showToast("Move undone");
        } else {
            showToast("Undo unavailable");
        }
        return;
    }

    if (renderer.isKeyRPressed()) {
        if (!game->getMoveHistory().empty()) {
            showConfirmModal = true;
        } else {
            game->reset();
            selectedPos.reset();
            legalDestinations.clear();
            showToast("Board reset");
        }
        return;
    }

    // Board click interaction
    if (renderer.isLeftMouseButtonPressed()) {
        int row = -1, col = -1;
        bool onBoard = renderer.getMouseBoardSquare(row, col, settings.flipped);

        if (!onBoard) {
            return;
        }

        Position clickedPos{ row, col };
        auto clickedPiece = game->getPieceAt(clickedPos);
        Color turn = game->getCurrentTurn();
        GameStatus status = game->getGameStatus();

        // If game is over, prevent further moves
        if (status == GameStatus::Checkmate || status == GameStatus::Stalemate) {
            showToast("Game over. Click Restart to play again.");
            selectedPos.reset();
            legalDestinations.clear();
            return;
        }

        if (!selectedPos.has_value()) {
            if (clickedPiece.has_value()) {
                if (clickedPiece->color == turn) {
                    selectedPos = clickedPos;
                    legalDestinations = game->getLegalDestinations(clickedPos);
                    showToast("Selected " + formatPieceName(clickedPiece->type, clickedPiece->color) + " at " + formatSquare(clickedPos));
                } else {
                    showToast(turn == Color::White ? "White's turn" : "Black's turn");
                }
            }
        } else {
            Position from = *selectedPos;
            if (from == clickedPos) {
                selectedPos.reset();
                legalDestinations.clear();
                showToast("Deselected");
            } else if (clickedPiece.has_value() && clickedPiece->color == turn) {
                // Switch piece selection to another piece of own color
                selectedPos = clickedPos;
                legalDestinations = game->getLegalDestinations(clickedPos);
                showToast("Selected " + formatPieceName(clickedPiece->type, clickedPiece->color) + " at " + formatSquare(clickedPos));
            } else {
                // Attempt move to destination
                if (game->isPromotionMove(from, clickedPos) && game->isLegalMove(from, clickedPos)) {
                    // Open promotion selection UI
                    pendingPromotion = std::make_pair(from, clickedPos);
                } else {
                    auto movingPiece = game->getPieceAt(from);
                    bool moved = game->makeMove(from, clickedPos);

                    if (moved) {
                        GameStatus newStatus = game->getGameStatus();
                        if (newStatus == GameStatus::Checkmate) {
                            std::string winner = (game->getWinner() == Color::White) ? "White" : "Black";
                            showToast("Checkmate! " + winner + " wins!");
                        } else if (newStatus == GameStatus::Stalemate) {
                            showToast("Stalemate! Game is a draw.");
                        } else if (newStatus == GameStatus::Check) {
                            showToast(formatPieceName(movingPiece->type, movingPiece->color) + " to " + formatSquare(clickedPos) + " (Check!)");
                        } else {
                            showToast(formatPieceName(movingPiece->type, movingPiece->color) + " to " + formatSquare(clickedPos));
                        }
                        selectedPos.reset();
                        legalDestinations.clear();
                        historyScrollOffset = 99999; // Auto-scroll to latest move
                    } else {
                        showToast("Illegal move rejected");
                    }
                }
            }
        }
    }
}

void BoardView::render() {
    int hoverRow = -1, hoverCol = -1;
    renderer.getMouseBoardSquare(hoverRow, hoverCol, settings.flipped);

    int selRow = selectedPos.has_value() ? selectedPos->row : -1;
    int selCol = selectedPos.has_value() ? selectedPos->col : -1;

    // Check king location
    int checkRow = -1, checkCol = -1;
    auto checkKing = game->getKingInCheckSquare();
    if (checkKing.has_value()) {
        checkRow = checkKing->row;
        checkCol = checkKing->col;
    }

    // Last move coordinates
    int lastFromRow = -1, lastFromCol = -1;
    int lastToRow = -1, lastToCol = -1;
    auto lastMove = game->getLastMove();
    if (lastMove.has_value()) {
        lastFromRow = lastMove->first.row;
        lastFromCol = lastMove->first.col;
        lastToRow = lastMove->second.row;
        lastToCol = lastMove->second.col;
    }

    renderer.beginDrawing();

    // 1. Draw chessboard background, tiles, and coordinates
    renderer.drawBoard(settings, selRow, selCol, hoverRow, hoverCol, checkRow, checkCol,
                       lastFromRow, lastFromCol, lastToRow, lastToCol);

    // 2. Draw legal destination hints if enabled
    if (settings.showMoveHints && selectedPos.has_value()) {
        for (const auto& dest : legalDestinations) {
            auto destPiece = game->getPieceAt(dest);
            bool isCapture = destPiece.has_value();
            renderer.drawLegalDestination(dest.row, dest.col, isCapture, settings.flipped);
        }
    }

    // 3. Draw all pieces currently on the board
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            auto pieceOpt = game->getPieceAt({ r, c });
            if (pieceOpt.has_value()) {
                char typeChar = 'P';
                switch (pieceOpt->type) {
                    case PieceType::Pawn:   typeChar = 'P'; break;
                    case PieceType::Rook:   typeChar = 'R'; break;
                    case PieceType::Knight: typeChar = 'N'; break;
                    case PieceType::Bishop: typeChar = 'B'; break;
                    case PieceType::Queen:  typeChar = 'Q'; break;
                    case PieceType::King:   typeChar = 'K'; break;
                }
                bool isWhite = (pieceOpt->color == Color::White);
                renderer.drawPiece(r, c, typeChar, isWhite, settings.flipped);
            }
        }
    }

    // 4. Draw sidebar header and player cards
    renderer.drawSidebarHeader();

    // Opponent player card (top) and user player card (bottom) based on flip orientation
    Color currentTurn = game->getCurrentTurn();
    Color topColor = settings.flipped ? Color::White : Color::Black;
    Color bottomColor = settings.flipped ? Color::Black : Color::White;

    auto toCharVec = [](const std::vector<PieceType>& pVec) {
        std::vector<char> res;
        for (auto p : pVec) {
            switch (p) {
                case PieceType::Pawn:   res.push_back('P'); break;
                case PieceType::Knight: res.push_back('N'); break;
                case PieceType::Bishop: res.push_back('B'); break;
                case PieceType::Rook:   res.push_back('R'); break;
                case PieceType::Queen:  res.push_back('Q'); break;
                default: break;
            }
        }
        return res;
    };

    renderer.drawPlayerCard(66, (topColor == Color::White ? "White" : "Black"),
                           (topColor == Color::White),
                           (currentTurn == topColor),
                           toCharVec(game->getCapturedPieces(topColor)),
                           game->getMaterialAdvantage(topColor));

    // 5. Status banner
    GameStatus status = game->getGameStatus();
    std::string statusText;
    int statusType = 0;

    if (status == GameStatus::Checkmate) {
        std::string winner = (game->getWinner() == Color::White) ? "White" : "Black";
        statusText = "Checkmate! " + winner + " wins!";
        statusType = 2;
    } else if (status == GameStatus::Stalemate) {
        statusText = "Draw by Stalemate";
        statusType = 3;
    } else if (status == GameStatus::Check) {
        statusText = (currentTurn == Color::White) ? "White is in check!" : "Black is in check!";
        statusType = 1;
    } else {
        statusText = (currentTurn == Color::White) ? "White to move" : "Black to move";
        statusType = 0;
    }

    renderer.drawStatusBanner(statusText, statusType);

    // 6. Move history panel
    std::vector<std::string> whiteMoves;
    std::vector<std::string> blackMoves;
    for (const auto& m : game->getMoveHistory()) {
        if (m.pieceColor == Color::White) {
            whiteMoves.push_back(m.notation);
        } else {
            blackMoves.push_back(m.notation);
        }
    }
    renderer.drawMoveHistory(whiteMoves, blackMoves, historyScrollOffset);

    // 7. Bottom player card
    int bottomCardY = renderer.getSidebarHeight() - 176;
    renderer.drawPlayerCard(bottomCardY, (bottomColor == Color::White ? "White" : "Black"),
                           (bottomColor == Color::White),
                           (currentTurn == bottomColor),
                           toCharVec(game->getCapturedPieces(bottomColor)),
                           game->getMaterialAdvantage(bottomColor));

    // 8. Control toolbar
    UIAction action = renderer.drawSidebarControls(game->canUndo());
    if (action == UIAction::NewGame || action == UIAction::Restart) {
        if (!game->getMoveHistory().empty()) {
            showConfirmModal = true;
        } else {
            game->reset();
            selectedPos.reset();
            legalDestinations.clear();
            showToast("New game started");
        }
    } else if (action == UIAction::Undo) {
        if (game->canUndo()) {
            game->undo();
            selectedPos.reset();
            legalDestinations.clear();
            showToast("Move undone");
        } else {
            showToast("Undo unavailable");
        }
    } else if (action == UIAction::FlipBoard) {
        settings.flipped = !settings.flipped;
        showToast(settings.flipped ? "Board flipped (Black perspective)" : "Board flipped (White perspective)");
    } else if (action == UIAction::OpenSettings) {
        showSettingsModal = true;
    }

    // 9. Active Modals & Dialogs
    if (pendingPromotion.has_value()) {
        bool isWhite = (game->getCurrentTurn() == Color::White);
        char promoChoice = renderer.drawPromotionModal(isWhite);
        if (promoChoice != '\0') {
            PieceType promoType = PieceType::Queen;
            if (promoChoice == 'R') promoType = PieceType::Rook;
            else if (promoChoice == 'B') promoType = PieceType::Bishop;
            else if (promoChoice == 'N') promoType = PieceType::Knight;

            Position from = pendingPromotion->first;
            Position to = pendingPromotion->second;
            bool moved = game->makeMove(from, to, promoType);
            if (moved) {
                showToast("Promoted to " + formatPieceName(promoType, isWhite ? Color::White : Color::Black));
                historyScrollOffset = 99999;
            }
            pendingPromotion.reset();
            selectedPos.reset();
            legalDestinations.clear();
        }
    } else if (showSettingsModal) {
        bool done = renderer.drawSettingsModal(settings);
        if (done) {
            showSettingsModal = false;
        }
    } else if (showConfirmModal) {
        ModalResult res = renderer.drawConfirmationModal("Reset Game", "Are you sure you want to reset the current game?");
        if (res == ModalResult::Confirm) {
            game->reset();
            selectedPos.reset();
            legalDestinations.clear();
            showConfirmModal = false;
            showToast("Game reset to initial position");
        } else if (res == ModalResult::Cancel) {
            showConfirmModal = false;
        }
    }

    // 10. Notification Toast
    if (toastTimer > 0.0f) {
        float alpha = std::min(1.0f, toastTimer * 2.0f);
        renderer.drawToast(toastMessage, alpha);
    }

    renderer.endDrawing();
}

// Backward compatibility methods
void BoardView::processInput(ChessEngine& /*engine*/) {
    processInput();
}

void BoardView::render(const ChessEngine& /*engine*/) {
    render();
}
