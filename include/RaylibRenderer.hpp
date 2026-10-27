#pragma once

#include "UITheme.hpp"
#include <memory>
#include <string>
#include <vector>

enum class UIAction {
    None,
    NewGame,
    Restart,
    Undo,
    FlipBoard,
    OpenSettings
};

enum class ModalResult {
    Pending,
    Confirm,
    Cancel
};

class RaylibRenderer {
private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;

public:
    RaylibRenderer(int width = 1100, int height = 750);
    ~RaylibRenderer();

    // Lifecycle
    void init(const char* title = "64 Squares");
    void close();
    bool shouldClose() const;

    // Window and Layout
    void updateLayout();
    int getWindowWidth() const;
    int getWindowHeight() const;
    int getSquareSize() const;
    int getBoardOffsetX() const;
    int getBoardOffsetY() const;
    int getSidebarX() const;
    int getSidebarY() const;
    int getSidebarWidth() const;
    int getSidebarHeight() const;

    // Frame rendering
    void beginDrawing();
    void endDrawing();
    void clearBackground();

    // Board & pieces rendering
    void drawBoard(const UISettings& settings,
                   int selectedRow = -1, int selectedCol = -1,
                   int hoverRow = -1, int hoverCol = -1,
                   int checkRow = -1, int checkCol = -1,
                   int lastFromRow = -1, int lastFromCol = -1,
                   int lastToRow = -1, int lastToCol = -1);

    void drawPiece(int row, int col, char type, bool isWhite, bool flipped);
    void drawLegalDestination(int row, int col, bool isCapture, bool flipped);

    // Sidebar & Components
    void drawSidebarHeader();
    void drawPlayerCard(int yOffset, const std::string& name, bool isWhite, bool isActive,
                        const std::vector<char>& capturedPieces, int materialAdvantage);
    void drawStatusBanner(const std::string& statusText, int statusType);
    void drawMoveHistory(const std::vector<std::string>& whiteMoves,
                         const std::vector<std::string>& blackMoves,
                         int& scrollOffset);
    UIAction drawSidebarControls(bool canUndo);

    // Modals & Dialogs
    char drawPromotionModal(bool isWhite);
    ModalResult drawConfirmationModal(const std::string& title, const std::string& message);
    bool drawSettingsModal(UISettings& settings);

    // Notification toast
    void drawToast(const std::string& message, float opacity);

    // Legacy method compatibility
    void drawUI(const std::string& turnText, bool isWhiteTurn, const std::string& statusMessage);

    // Input polling
    bool getMouseBoardSquare(int& outRow, int& outCol, bool flipped = false) const;
    bool isLeftMouseButtonPressed() const;
    bool isRightMouseButtonPressed() const;
    bool isKeyRPressed() const;
    bool isKeyFPressed() const;
    bool isKeyUPressed() const;
    bool isKeyEscapePressed() const;
    float getMouseWheelMove() const;
};
