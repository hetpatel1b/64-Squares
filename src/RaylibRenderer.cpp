#include "RaylibRenderer.hpp"
#include <raylib.h>
#include <map>
#include <vector>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

struct RaylibRenderer::Impl {
    int windowWidth;
    int windowHeight;
    int boardOffsetX;
    int boardOffsetY;
    int squareSize;

    std::map<std::string, Texture2D> textures;
    bool texturesLoaded = false;
    bool initialized = false;

    Impl(int width, int height)
        : windowWidth(width),
          windowHeight(height),
          boardOffsetX(80),
          boardOffsetY(90),
          squareSize(80) {}

    std::string resolveAssetPath(const std::string& relativePath) {
        std::vector<std::string> candidates = {
            relativePath,
            "../" + relativePath,
            "../../" + relativePath
        };

        const char* appDir = GetApplicationDirectory();
        if (appDir != nullptr && appDir[0] != '\0') {
            candidates.push_back(std::string(appDir) + relativePath);
            candidates.push_back(std::string(appDir) + "../" + relativePath);
        }

        for (const auto& path : candidates) {
            std::error_code ec;
            if (fs::exists(path, ec) && fs::is_regular_file(path, ec)) {
                return path;
            }
        }
        return "";
    }

    void loadTextures() {
        if (texturesLoaded) return;

        const std::vector<std::string> pieceKeys = {
            "w_pawn", "w_rook", "w_knight", "w_bishop", "w_queen", "w_king",
            "b_pawn", "b_rook", "b_knight", "b_bishop", "b_queen", "b_king"
        };

        for (const auto& key : pieceKeys) {
            std::string path = resolveAssetPath("assets/pieces/" + key + ".png");
            if (!path.empty()) {
                Texture2D tex = LoadTexture(path.c_str());
                if (tex.id > 0) {
                    textures[key] = tex;
                }
            }
        }

        texturesLoaded = true;
    }

    void unloadTextures() {
        if (!texturesLoaded) return;
        for (auto& pair : textures) {
            if (pair.second.id > 0) {
                UnloadTexture(pair.second);
            }
        }
        textures.clear();
        texturesLoaded = false;
    }
};

RaylibRenderer::RaylibRenderer(int width, int height)
    : pImpl(std::make_unique<Impl>(width, height)) {}

RaylibRenderer::~RaylibRenderer() {
    close();
}

void RaylibRenderer::init(const char* title) {
    if (pImpl->initialized) return;

    InitWindow(pImpl->windowWidth, pImpl->windowHeight, title);
    SetTargetFPS(60);
    pImpl->loadTextures();
    pImpl->initialized = true;
}

void RaylibRenderer::close() {
    if (!pImpl->initialized) return;

    pImpl->unloadTextures();
    if (IsWindowReady()) {
        CloseWindow();
    }
    pImpl->initialized = false;
}

bool RaylibRenderer::shouldClose() const {
    return WindowShouldClose();
}

void RaylibRenderer::beginDrawing() {
    BeginDrawing();
}

void RaylibRenderer::endDrawing() {
    EndDrawing();
}

void RaylibRenderer::drawBoard(int selectedRow, int selectedCol, int hoverRow, int hoverCol) {
    ClearBackground(CLITERAL(Color){ 24, 26, 32, 255 });

    const Color lightSquare = CLITERAL(Color){ 240, 217, 181, 255 };
    const Color darkSquare  = CLITERAL(Color){ 181, 136, 99, 255 };
    const Color boardBorder = CLITERAL(Color){ 60, 48, 38, 255 };

    // Outer border around board
    DrawRectangleLinesEx(
        CLITERAL(Rectangle){
            static_cast<float>(pImpl->boardOffsetX - 3),
            static_cast<float>(pImpl->boardOffsetY - 3),
            static_cast<float>(8 * pImpl->squareSize + 6),
            static_cast<float>(8 * pImpl->squareSize + 6)
        },
        3.0f,
        boardBorder
    );

    // 8x8 alternating squares
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            int sqX = pImpl->boardOffsetX + c * pImpl->squareSize;
            int sqY = pImpl->boardOffsetY + r * pImpl->squareSize;

            bool isLight = ((r + c) % 2 == 0);
            Color sqColor = isLight ? lightSquare : darkSquare;

            DrawRectangle(sqX, sqY, pImpl->squareSize, pImpl->squareSize, sqColor);

            // Hover highlight
            if (r == hoverRow && c == hoverCol) {
                DrawRectangle(sqX, sqY, pImpl->squareSize, pImpl->squareSize,
                              CLITERAL(Color){ 255, 255, 255, 40 });
            }

            // Selection highlight
            if (r == selectedRow && c == selectedCol) {
                DrawRectangle(sqX, sqY, pImpl->squareSize, pImpl->squareSize,
                              CLITERAL(Color){ 255, 220, 60, 150 });
                DrawRectangleLinesEx(
                    CLITERAL(Rectangle){
                        static_cast<float>(sqX),
                        static_cast<float>(sqY),
                        static_cast<float>(pImpl->squareSize),
                        static_cast<float>(pImpl->squareSize)
                    },
                    3.0f,
                    CLITERAL(Color){ 245, 195, 0, 255 }
                );
            }
        }
    }

    // Rank coordinates (8 to 1) on left side
    const char* rankLabels[] = { "8", "7", "6", "5", "4", "3", "2", "1" };
    for (int r = 0; r < 8; ++r) {
        int textY = pImpl->boardOffsetY + r * pImpl->squareSize + (pImpl->squareSize - 16) / 2;
        int textX = pImpl->boardOffsetX - 28;
        DrawText(rankLabels[r], textX, textY, 18, CLITERAL(Color){ 175, 180, 192, 255 });
    }

    // File coordinates (A to H) on bottom side
    const char* fileLabels[] = { "A", "B", "C", "D", "E", "F", "G", "H" };
    for (int c = 0; c < 8; ++c) {
        int textX = pImpl->boardOffsetX + c * pImpl->squareSize + (pImpl->squareSize - 12) / 2;
        int textY = pImpl->boardOffsetY + 8 * pImpl->squareSize + 10;
        DrawText(fileLabels[c], textX, textY, 18, CLITERAL(Color){ 175, 180, 192, 255 });
    }
}

void RaylibRenderer::drawPiece(int row, int col, char type, bool isWhite) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return;

    int sqX = pImpl->boardOffsetX + col * pImpl->squareSize;
    int sqY = pImpl->boardOffsetY + row * pImpl->squareSize;

    // Check if texture asset exists
    std::string key = (isWhite ? "w_" : "b_");
    switch (type) {
        case 'P': key += "pawn"; break;
        case 'R': key += "rook"; break;
        case 'N': key += "knight"; break;
        case 'B': key += "bishop"; break;
        case 'Q': key += "queen"; break;
        case 'K': key += "king"; break;
        default: return;
    }

    auto it = pImpl->textures.find(key);
    if (it != pImpl->textures.end() && it->second.id > 0) {
        Rectangle srcRec = { 0.0f, 0.0f, static_cast<float>(it->second.width), static_cast<float>(it->second.height) };
        Rectangle dstRec = {
            static_cast<float>(sqX + 8),
            static_cast<float>(sqY + 8),
            static_cast<float>(pImpl->squareSize - 16),
            static_cast<float>(pImpl->squareSize - 16)
        };
        DrawTexturePro(it->second, srcRec, dstRec, CLITERAL(Vector2){ 0, 0 }, 0.0f, WHITE);
        return;
    }

    // Procedural piece rendering fallback
    float centerX = sqX + pImpl->squareSize / 2.0f;
    float centerY = sqY + pImpl->squareSize / 2.0f - 2.0f;
    float radius  = pImpl->squareSize * 0.35f;

    Color fillCol   = isWhite ? CLITERAL(Color){ 252, 250, 244, 255 } : CLITERAL(Color){ 36, 40, 48, 255 };
    Color borderCol = isWhite ? CLITERAL(Color){ 50, 52, 60, 255 }    : CLITERAL(Color){ 215, 220, 230, 255 };
    Color textCol   = isWhite ? CLITERAL(Color){ 25, 28, 35, 255 }    : CLITERAL(Color){ 245, 245, 250, 255 };

    // Piece token badge
    DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, fillCol);
    DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), radius, borderCol);

    // Piece character
    char charStr[2] = { type, '\0' };
    int fontSize = 32;
    int textW = MeasureText(charStr, fontSize);
    DrawText(charStr, static_cast<int>(centerX - textW / 2.0f), static_cast<int>(centerY - fontSize / 2.0f + 1), fontSize, textCol);
}

void RaylibRenderer::drawUI(const std::string& turnText, bool isWhiteTurn, const std::string& statusMessage) {
    // Header title
    DrawText("64 SQUARES", pImpl->boardOffsetX, 22, 24, CLITERAL(Color){ 240, 242, 248, 255 });

    // Turn indicator badge
    int turnX = pImpl->boardOffsetX + 220;
    int turnY = 34;
    Color turnBadgeColor = isWhiteTurn ? CLITERAL(Color){ 250, 250, 250, 255 } : CLITERAL(Color){ 35, 38, 45, 255 };
    DrawCircle(turnX, turnY, 9.0f, turnBadgeColor);
    DrawCircleLines(turnX, turnY, 9.0f, CLITERAL(Color){ 180, 185, 195, 255 });

    DrawText(turnText.c_str(), turnX + 16, 25, 18, CLITERAL(Color){ 220, 225, 235, 255 });

    // Status message
    if (!statusMessage.empty()) {
        int msgW = MeasureText(statusMessage.c_str(), 16);
        int rightEdge = pImpl->boardOffsetX + 8 * pImpl->squareSize;
        int msgX = rightEdge - msgW;
        if (msgX < turnX + 160) msgX = turnX + 160;
        DrawText(statusMessage.c_str(), msgX, 27, 16, CLITERAL(Color){ 255, 205, 100, 255 });
    }

    // Subdued footer instructions
    const char* hint = "Click piece then destination  |  [R] Reset Board  |  [ESC] Deselect";
    int hintW = MeasureText(hint, 13);
    int hintX = pImpl->boardOffsetX + (8 * pImpl->squareSize - hintW) / 2;
    DrawText(hint, hintX, pImpl->boardOffsetY + 8 * pImpl->squareSize + 40, 13, CLITERAL(Color){ 130, 135, 145, 255 });
}

bool RaylibRenderer::getMouseBoardSquare(int& outRow, int& outCol) const {
    Vector2 m = GetMousePosition();
    int col = static_cast<int>((m.x - pImpl->boardOffsetX) / pImpl->squareSize);
    int row = static_cast<int>((m.y - pImpl->boardOffsetY) / pImpl->squareSize);

    if (m.x >= pImpl->boardOffsetX &&
        m.y >= pImpl->boardOffsetY &&
        col >= 0 && col < 8 &&
        row >= 0 && row < 8) {
        outRow = row;
        outCol = col;
        return true;
    }
    return false;
}

bool RaylibRenderer::isLeftMouseButtonPressed() const {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool RaylibRenderer::isRightMouseButtonPressed() const {
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

bool RaylibRenderer::isKeyRPressed() const {
    return IsKeyPressed(KEY_R);
}

bool RaylibRenderer::isKeyEscapePressed() const {
    return IsKeyPressed(KEY_ESCAPE);
}

int RaylibRenderer::getSquareSize() const {
    return pImpl->squareSize;
}

int RaylibRenderer::getBoardOffsetX() const {
    return pImpl->boardOffsetX;
}

int RaylibRenderer::getBoardOffsetY() const {
    return pImpl->boardOffsetY;
}
