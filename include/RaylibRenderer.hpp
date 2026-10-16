#pragma once

#include <memory>
#include <string>

class RaylibRenderer {
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

public:
    RaylibRenderer(int width = 800, int height = 800);
    ~RaylibRenderer();

    // Lifecycle
    void init(const char* title = "64 Squares");
    void close();
    bool shouldClose() const;

    // Frame rendering
    void beginDrawing();
    void endDrawing();

    // Board & pieces rendering
    void drawBoard(int selectedRow = -1, int selectedCol = -1, int hoverRow = -1, int hoverCol = -1);
    void drawPiece(int row, int col, char type, bool isWhite);
    void drawUI(const std::string& turnText, bool isWhiteTurn, const std::string& statusMessage);

    // Input polling
    bool getMouseBoardSquare(int& outRow, int& outCol) const;
    bool isLeftMouseButtonPressed() const;
    bool isRightMouseButtonPressed() const;
    bool isKeyRPressed() const;
    bool isKeyEscapePressed() const;

    int getSquareSize() const;
    int getBoardOffsetX() const;
    int getBoardOffsetY() const;
};
