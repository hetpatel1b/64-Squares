#pragma once

#include "IChessGame.hpp"
#include "RaylibRenderer.hpp"
#include "UITheme.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @brief Main UI controller and view model managing the presentation layer,
 * user interactions, layout responsiveness, and engine dispatching.
 */
class BoardView {
private:
    std::shared_ptr<IChessGame> game;
    RaylibRenderer renderer;
    UISettings settings;

    // Interactive UI state
    std::optional<Position> selectedPos;
    std::vector<Position> legalDestinations;

    // Dialog & Modal state
    bool showSettingsModal{false};
    bool showConfirmModal{false};
    std::optional<std::pair<Position, Position>> pendingPromotion;

    // Notification toast
    std::string toastMessage{"White to move"};
    float toastTimer{2.5f};

    // Scroll state for move history
    int historyScrollOffset{0};

    // Formatting helpers
    std::string formatSquare(Position pos) const;
    std::string formatPieceName(PieceType type, Color color) const;

public:
    explicit BoardView(int width = 1100, int height = 750);
    explicit BoardView(std::shared_ptr<IChessGame> engineGame, int width = 1100, int height = 750);
    ~BoardView() = default;

    void init();
    void cleanup();
    bool shouldClose() const;

    void setEngine(std::shared_ptr<IChessGame> engineGame);
    std::shared_ptr<IChessGame> getEngine() const { return game; }

    void showToast(const std::string& message, float duration = 2.5f);

    void processInput();
    void render();

    // Compatibility methods for existing main.cpp loop
    void processInput(class ChessEngine& engine);
    void render(const class ChessEngine& engine);
};
