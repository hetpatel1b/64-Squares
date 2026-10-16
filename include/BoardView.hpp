#pragma once

#include "ChessEngine.hpp"
#include "RaylibRenderer.hpp"
#include <optional>
#include <string>

class BoardView {
private:
    RaylibRenderer renderer;
    std::optional<Position> selectedPos;
    std::string statusMessage;

    std::string formatSquare(Position pos) const;
    std::string formatPieceName(const Piece* piece) const;

public:
    BoardView(int width = 800, int height = 800);
    ~BoardView() = default;

    void init();
    void cleanup();
    bool shouldClose() const;

    void processInput(ChessEngine& engine);
    void render(const ChessEngine& engine);
};
