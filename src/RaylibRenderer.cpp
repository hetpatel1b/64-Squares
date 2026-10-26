#include "RaylibRenderer.hpp"
#include <raylib.h>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

struct RaylibRenderer::Impl {
    int windowWidth;
    int windowHeight;
    int boardOffsetX{50};
    int boardOffsetY{50};
    int squareSize{75};
    int sidebarX{680};
    int sidebarY{20};
    int sidebarWidth{340};
    int sidebarHeight{710};

    std::map<std::string, Texture2D> textures;
    bool texturesLoaded{false};
    bool initialized{false};

    Impl(int width, int height)
        : windowWidth(width),
          windowHeight(height) {
        updateDimensions(width, height);
    }

    void updateDimensions(int width, int height) {
        windowWidth = width;
        windowHeight = height;

        // Dynamic sidebar layout
        sidebarWidth = std::clamp(static_cast<int>(width * 0.32f), 310, 380);
        sidebarX = width - sidebarWidth - 18;
        sidebarY = 18;
        sidebarHeight = height - 36;

        // Board layout centered in the remaining left viewport
        int availableBoardW = sidebarX - 40;
        int availableBoardH = height - 50;
        int maxBoard = std::max(280, std::min(availableBoardW, availableBoardH));

        squareSize = maxBoard / 8;
        int boardTotal = squareSize * 8;
        boardOffsetX = std::max(20, (sidebarX - boardTotal) / 2);
        boardOffsetY = std::max(18, (height - boardTotal) / 2);
    }

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

    // Helper for interactive button rendering
    bool drawButton(Rectangle bounds, const char* text, bool disabled, bool active = false) {
        Vector2 mouse = GetMousePosition();
        bool hovered = !disabled && CheckCollisionPointRec(mouse, bounds);
        bool pressed = hovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool clicked = hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

        Color bgCol = CLITERAL(Color){ 36, 42, 54, 255 };
        Color borderCol = CLITERAL(Color){ 55, 65, 82, 255 };
        Color textCol = CLITERAL(Color){ 215, 222, 235, 255 };

        if (disabled) {
            bgCol = CLITERAL(Color){ 24, 28, 36, 255 };
            borderCol = CLITERAL(Color){ 38, 44, 56, 255 };
            textCol = CLITERAL(Color){ 90, 100, 120, 255 };
        } else if (pressed) {
            bgCol = CLITERAL(Color){ 26, 30, 40, 255 };
            borderCol = CLITERAL(Color){ 90, 120, 160, 255 };
            textCol = CLITERAL(Color){ 255, 255, 255, 255 };
        } else if (hovered) {
            bgCol = CLITERAL(Color){ 48, 58, 76, 255 };
            borderCol = CLITERAL(Color){ 100, 140, 190, 255 };
            textCol = CLITERAL(Color){ 255, 255, 255, 255 };
        } else if (active) {
            bgCol = CLITERAL(Color){ 44, 62, 90, 255 };
            borderCol = CLITERAL(Color){ 110, 155, 220, 255 };
            textCol = CLITERAL(Color){ 255, 255, 255, 255 };
        }

        DrawRectangleRounded(bounds, 0.22f, 4, bgCol);
        DrawRectangleRoundedLinesEx(bounds, 0.22f, 4, 1.5f, borderCol);

        int fontSize = 14;
        int textW = MeasureText(text, fontSize);
        int textX = static_cast<int>(bounds.x + (bounds.width - textW) / 2.0f);
        int textY = static_cast<int>(bounds.y + (bounds.height - fontSize) / 2.0f);
        DrawText(text, textX, textY, fontSize, textCol);

        return clicked;
    }

    // High quality procedural piece drawing
    void drawProceduralPiece(float centerX, float centerY, float sqSize, char type, bool isWhite) {
        float r = sqSize * 0.36f;

        // Visual color palette
        Color pieceBody = isWhite ? CLITERAL(Color){ 248, 246, 240, 255 } : CLITERAL(Color){ 28, 32, 40, 255 };
        Color pieceOutline = isWhite ? CLITERAL(Color){ 42, 46, 56, 255 } : CLITERAL(Color){ 150, 160, 180, 255 };
        Color glyphColor = isWhite ? CLITERAL(Color){ 30, 34, 44, 255 } : CLITERAL(Color){ 240, 244, 250, 255 };
        Color shadowColor = CLITERAL(Color){ 10, 12, 16, 75 };

        // Soft drop shadow
        DrawCircle(static_cast<int>(centerX + 2.0f), static_cast<int>(centerY + 3.0f), r, shadowColor);

        // Piece Token Base
        DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), r, pieceBody);
        DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), r, pieceOutline);

        // Subtle inner ring highlight for depth
        DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), r * 0.86f,
                        isWhite ? CLITERAL(Color){ 255, 255, 255, 120 } : CLITERAL(Color){ 50, 56, 70, 180 });

        // Geometric symbol glyph
        float scale = sqSize / 80.0f;
        switch (type) {
            case 'P': { // Pawn
                float topY = centerY - 9.0f * scale;
                DrawCircle(static_cast<int>(centerX), static_cast<int>(topY), 6.5f * scale, glyphColor);
                Rectangle base = { centerX - 8.0f * scale, centerY + 3.0f * scale, 16.0f * scale, 5.0f * scale };
                DrawRectangleRounded(base, 0.4f, 2, glyphColor);
                DrawTriangle(
                    CLITERAL(Vector2){ centerX - 5.0f * scale, centerY + 3.0f * scale },
                    CLITERAL(Vector2){ centerX + 5.0f * scale, centerY + 3.0f * scale },
                    CLITERAL(Vector2){ centerX, topY + 4.0f * scale },
                    glyphColor
                );
                break;
            }
            case 'R': { // Rook (Castle battlements)
                Rectangle base = { centerX - 12.0f * scale, centerY + 6.0f * scale, 24.0f * scale, 5.0f * scale };
                DrawRectangleRounded(base, 0.3f, 2, glyphColor);
                Rectangle tower = { centerX - 9.0f * scale, centerY - 6.0f * scale, 18.0f * scale, 12.0f * scale };
                DrawRectangleRounded(tower, 0.1f, 2, glyphColor);
                // 3 battlements
                DrawRectangle(static_cast<int>(centerX - 10.0f * scale), static_cast<int>(centerY - 13.0f * scale), static_cast<int>(5.0f * scale), static_cast<int>(7.0f * scale), glyphColor);
                DrawRectangle(static_cast<int>(centerX - 2.5f * scale), static_cast<int>(centerY - 13.0f * scale), static_cast<int>(5.0f * scale), static_cast<int>(7.0f * scale), glyphColor);
                DrawRectangle(static_cast<int>(centerX + 5.0f * scale), static_cast<int>(centerY - 13.0f * scale), static_cast<int>(5.0f * scale), static_cast<int>(7.0f * scale), glyphColor);
                break;
            }
            case 'N': { // Knight (Horse profile)
                DrawCircle(static_cast<int>(centerX - 2.0f * scale), static_cast<int>(centerY - 7.0f * scale), 7.0f * scale, glyphColor);
                // Snout / muzzle
                DrawTriangle(
                    CLITERAL(Vector2){ centerX - 12.0f * scale, centerY - 4.0f * scale },
                    CLITERAL(Vector2){ centerX - 3.0f * scale, centerY - 10.0f * scale },
                    CLITERAL(Vector2){ centerX - 3.0f * scale, centerY },
                    glyphColor
                );
                // Ear
                DrawTriangle(
                    CLITERAL(Vector2){ centerX, centerY - 8.0f * scale },
                    CLITERAL(Vector2){ centerX + 5.0f * scale, centerY - 15.0f * scale },
                    CLITERAL(Vector2){ centerX + 5.0f * scale, centerY - 7.0f * scale },
                    glyphColor
                );
                // Body & Base
                Rectangle body = { centerX - 6.0f * scale, centerY - 2.0f * scale, 14.0f * scale, 10.0f * scale };
                DrawRectangleRounded(body, 0.3f, 2, glyphColor);
                Rectangle base = { centerX - 11.0f * scale, centerY + 7.0f * scale, 22.0f * scale, 5.0f * scale };
                DrawRectangleRounded(base, 0.4f, 2, glyphColor);
                break;
            }
            case 'B': { // Bishop (Mitre)
                DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY - 13.0f * scale), 3.0f * scale, glyphColor);
                Rectangle base = { centerX - 10.0f * scale, centerY + 7.0f * scale, 20.0f * scale, 4.5f * scale };
                DrawRectangleRounded(base, 0.4f, 2, glyphColor);
                // Mitre head
                DrawTriangle(
                    CLITERAL(Vector2){ centerX - 8.0f * scale, centerY + 6.0f * scale },
                    CLITERAL(Vector2){ centerX + 8.0f * scale, centerY + 6.0f * scale },
                    CLITERAL(Vector2){ centerX, centerY - 11.0f * scale },
                    glyphColor
                );
                // Slit line
                DrawLineEx(
                    CLITERAL(Vector2){ centerX - 3.0f * scale, centerY - 5.0f * scale },
                    CLITERAL(Vector2){ centerX + 3.0f * scale, centerY + 1.0f * scale },
                    2.0f * scale,
                    pieceBody
                );
                break;
            }
            case 'Q': { // Queen (Coronet with pearls)
                Rectangle base = { centerX - 12.0f * scale, centerY + 7.0f * scale, 24.0f * scale, 5.0f * scale };
                DrawRectangleRounded(base, 0.4f, 2, glyphColor);
                DrawTriangle(
                    CLITERAL(Vector2){ centerX - 10.0f * scale, centerY + 6.0f * scale },
                    CLITERAL(Vector2){ centerX + 10.0f * scale, centerY + 6.0f * scale },
                    CLITERAL(Vector2){ centerX, centerY - 3.0f * scale },
                    glyphColor
                );
                // Coronet tips
                float xOffsets[] = { -9.0f, -4.5f, 0.0f, 4.5f, 9.0f };
                float yOffsets[] = { -8.0f, -12.0f, -14.0f, -12.0f, -8.0f };
                for (int i = 0; i < 5; ++i) {
                    DrawCircle(static_cast<int>(centerX + xOffsets[i] * scale),
                               static_cast<int>(centerY + yOffsets[i] * scale),
                               2.8f * scale, glyphColor);
                }
                break;
            }
            case 'K': { // King (Imperial Cross)
                Rectangle base = { centerX - 12.0f * scale, centerY + 7.0f * scale, 24.0f * scale, 5.0f * scale };
                DrawRectangleRounded(base, 0.4f, 2, glyphColor);
                // Crown body
                Rectangle crown = { centerX - 9.0f * scale, centerY - 3.0f * scale, 18.0f * scale, 10.0f * scale };
                DrawRectangleRounded(crown, 0.3f, 2, glyphColor);
                // Cross finial
                DrawRectangle(static_cast<int>(centerX - 2.0f * scale), static_cast<int>(centerY - 15.0f * scale), static_cast<int>(4.0f * scale), static_cast<int>(12.0f * scale), glyphColor);
                DrawRectangle(static_cast<int>(centerX - 6.0f * scale), static_cast<int>(centerY - 12.0f * scale), static_cast<int>(12.0f * scale), static_cast<int>(4.0f * scale), glyphColor);
                break;
            }
            default:
                break;
        }
    }
};

RaylibRenderer::RaylibRenderer(int width, int height)
    : pImpl(std::make_unique<Impl>(width, height)) {}

RaylibRenderer::~RaylibRenderer() {
    close();
}

void RaylibRenderer::init(const char* title) {
    if (pImpl->initialized) return;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(pImpl->windowWidth, pImpl->windowHeight, title);
    SetWindowMinSize(880, 620);
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

void RaylibRenderer::updateLayout() {
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    pImpl->updateDimensions(w, h);
}

int RaylibRenderer::getWindowWidth() const { return pImpl->windowWidth; }
int RaylibRenderer::getWindowHeight() const { return pImpl->windowHeight; }
int RaylibRenderer::getSquareSize() const { return pImpl->squareSize; }
int RaylibRenderer::getBoardOffsetX() const { return pImpl->boardOffsetX; }
int RaylibRenderer::getBoardOffsetY() const { return pImpl->boardOffsetY; }
int RaylibRenderer::getSidebarX() const { return pImpl->sidebarX; }
int RaylibRenderer::getSidebarY() const { return pImpl->sidebarY; }
int RaylibRenderer::getSidebarWidth() const { return pImpl->sidebarWidth; }
int RaylibRenderer::getSidebarHeight() const { return pImpl->sidebarHeight; }

void RaylibRenderer::beginDrawing() {
    BeginDrawing();
}

void RaylibRenderer::endDrawing() {
    EndDrawing();
}

void RaylibRenderer::clearBackground() {
    ClearBackground(CLITERAL(Color){ 18, 20, 26, 255 });
}

void RaylibRenderer::drawBoard(const UISettings& settings,
                             int selectedRow, int selectedCol,
                             int hoverRow, int hoverCol,
                             int checkRow, int checkCol,
                             int lastFromRow, int lastFromCol,
                             int lastToRow, int lastToCol) {
    clearBackground();

    // Select color palette based on theme
    Color lightSquareCol, darkSquareCol, borderCol;
    switch (settings.theme) {
        case BoardTheme::ModernSlate:
            lightSquareCol = CLITERAL(Color){ 222, 227, 235, 255 };
            darkSquareCol  = CLITERAL(Color){ 112, 132, 153, 255 };
            borderCol      = CLITERAL(Color){ 38, 46, 58, 255 };
            break;
        case BoardTheme::ForestGreen:
            lightSquareCol = CLITERAL(Color){ 235, 238, 222, 255 };
            darkSquareCol  = CLITERAL(Color){ 115, 149, 82, 255 };
            borderCol      = CLITERAL(Color){ 36, 48, 30, 255 };
            break;
        case BoardTheme::MidnightBlue:
            lightSquareCol = CLITERAL(Color){ 218, 226, 236, 255 };
            darkSquareCol  = CLITERAL(Color){ 51, 78, 104, 255 };
            borderCol      = CLITERAL(Color){ 24, 34, 46, 255 };
            break;
        case BoardTheme::ClassicWood:
        default:
            lightSquareCol = CLITERAL(Color){ 240, 217, 181, 255 };
            darkSquareCol  = CLITERAL(Color){ 181, 136, 99, 255 };
            borderCol      = CLITERAL(Color){ 58, 44, 34, 255 };
            break;
    }

    int sq = pImpl->squareSize;
    int bX = pImpl->boardOffsetX;
    int bY = pImpl->boardOffsetY;
    int bTotal = sq * 8;

    // Outer subtle board border with rounded bevel
    Rectangle boardOuter = {
        static_cast<float>(bX - 6),
        static_cast<float>(bY - 6),
        static_cast<float>(bTotal + 12),
        static_cast<float>(bTotal + 12)
    };
    DrawRectangleRounded(boardOuter, 0.02f, 4, borderCol);

    // Board inner shadow
    DrawRectangle(bX - 2, bY - 2, bTotal + 4, bTotal + 4, CLITERAL(Color){ 12, 14, 18, 120 });

    // 8x8 alternating grid
    for (int dispR = 0; dispR < 8; ++dispR) {
        for (int dispC = 0; dispC < 8; ++dispC) {
            int r = settings.flipped ? (7 - dispR) : dispR;
            int c = settings.flipped ? (7 - dispC) : dispC;

            int cellX = bX + dispC * sq;
            int cellY = bY + dispR * sq;

            bool isLight = ((r + c) % 2 == 0);
            Color sqCol = isLight ? lightSquareCol : darkSquareCol;
            DrawRectangle(cellX, cellY, sq, sq, sqCol);

            // Previous move highlight
            if (settings.highlightLastMove) {
                if ((r == lastFromRow && c == lastFromCol) ||
                    (r == lastToRow && c == lastToCol)) {
                    DrawRectangle(cellX, cellY, sq, sq, CLITERAL(Color){ 205, 210, 70, 110 });
                }
            }

            // King in check highlight
            if (r == checkRow && c == checkCol) {
                DrawRectangle(cellX, cellY, sq, sq, CLITERAL(Color){ 235, 50, 50, 160 });
                DrawRectangleLinesEx(
                    CLITERAL(Rectangle){ static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(sq), static_cast<float>(sq) },
                    3.0f,
                    CLITERAL(Color){ 255, 30, 30, 255 }
                );
            }

            // Selected square highlight
            if (r == selectedRow && c == selectedCol) {
                DrawRectangle(cellX, cellY, sq, sq, CLITERAL(Color){ 245, 197, 24, 140 });
                DrawRectangleLinesEx(
                    CLITERAL(Rectangle){ static_cast<float>(cellX), static_cast<float>(cellY), static_cast<float>(sq), static_cast<float>(sq) },
                    3.0f,
                    CLITERAL(Color){ 255, 215, 0, 255 }
                );
            }

            // Hover highlight
            if (r == hoverRow && c == hoverCol && !(r == selectedRow && c == selectedCol)) {
                DrawRectangle(cellX, cellY, sq, sq, CLITERAL(Color){ 255, 255, 255, 45 });
            }

            // Board algebraic coordinates
            if (settings.showCoordinates) {
                Color coordCol = isLight ? CLITERAL(Color){ 140, 110, 80, 220 } : CLITERAL(Color){ 230, 220, 205, 220 };
                if (settings.theme != BoardTheme::ClassicWood) {
                    coordCol = isLight ? CLITERAL(Color){ 80, 90, 105, 220 } : CLITERAL(Color){ 220, 225, 235, 220 };
                }

                // File letter on bottom-most square row
                if (dispR == 7) {
                    char fileChar = static_cast<char>('a' + c);
                    char fileStr[2] = { fileChar, '\0' };
                    DrawText(fileStr, cellX + sq - 13, cellY + sq - 15, 12, coordCol);
                }

                // Rank digit on left-most square col
                if (dispC == 0) {
                    char rankChar = static_cast<char>('8' - r);
                    char rankStr[2] = { rankChar, '\0' };
                    DrawText(rankStr, cellX + 3, cellY + 3, 12, coordCol);
                }
            }
        }
    }
}

void RaylibRenderer::drawPiece(int row, int col, char type, bool isWhite, bool flipped) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) return;

    int dispR = flipped ? (7 - row) : row;
    int dispC = flipped ? (7 - col) : col;

    int sq = pImpl->squareSize;
    int cellX = pImpl->boardOffsetX + dispC * sq;
    int cellY = pImpl->boardOffsetY + dispR * sq;

    // Check texture availability
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
            static_cast<float>(cellX + 6),
            static_cast<float>(cellY + 6),
            static_cast<float>(sq - 12),
            static_cast<float>(sq - 12)
        };
        DrawTexturePro(it->second, srcRec, dstRec, CLITERAL(Vector2){ 0, 0 }, 0.0f, WHITE);
        return;
    }

    // Procedural rendering fallback
    float centerX = cellX + sq / 2.0f;
    float centerY = cellY + sq / 2.0f;
    pImpl->drawProceduralPiece(centerX, centerY, static_cast<float>(sq), type, isWhite);
}

void RaylibRenderer::drawLegalDestination(int row, int col, bool isCapture, bool flipped) {
    int dispR = flipped ? (7 - row) : row;
    int dispC = flipped ? (7 - col) : col;

    int sq = pImpl->squareSize;
    float centerX = pImpl->boardOffsetX + dispC * sq + sq / 2.0f;
    float centerY = pImpl->boardOffsetY + dispR * sq + sq / 2.0f;

    if (isCapture) {
        // Hollow ring around captured piece
        float outerRadius = sq * 0.42f;
        float innerRadius = sq * 0.34f;
        DrawRing(CLITERAL(Vector2){ centerX, centerY }, innerRadius, outerRadius, 0.0f, 360.0f, 32, CLITERAL(Color){ 245, 197, 24, 180 });
    } else {
        // Central dot on empty destination
        float dotRadius = sq * 0.14f;
        DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), dotRadius, CLITERAL(Color){ 30, 35, 45, 120 });
        DrawCircleLines(static_cast<int>(centerX), static_cast<int>(centerY), dotRadius, CLITERAL(Color){ 255, 255, 255, 100 });
    }
}

void RaylibRenderer::drawSidebarHeader() {
    Rectangle barRect = {
        static_cast<float>(pImpl->sidebarX),
        static_cast<float>(pImpl->sidebarY),
        static_cast<float>(pImpl->sidebarWidth),
        static_cast<float>(pImpl->sidebarHeight)
    };

    // Main dark sidebar container
    DrawRectangleRounded(barRect, 0.02f, 4, CLITERAL(Color){ 24, 27, 34, 255 });
    DrawRectangleRoundedLinesEx(barRect, 0.02f, 4, 1.5f, CLITERAL(Color){ 44, 50, 64, 255 });

    // App header title
    int titleX = pImpl->sidebarX + 20;
    int titleY = pImpl->sidebarY + 18;
    DrawText("64 SQUARES", titleX, titleY, 20, CLITERAL(Color){ 245, 248, 255, 255 });

    int subY = titleY + 23;
    DrawText("CHESS DESKTOP  v1.0", titleX, subY, 11, CLITERAL(Color){ 120, 132, 154, 255 });

    // Subtle header divider
    DrawLine(titleX, subY + 16, pImpl->sidebarX + pImpl->sidebarWidth - 20, subY + 16, CLITERAL(Color){ 40, 46, 58, 255 });
}

void RaylibRenderer::drawPlayerCard(int yOffset, const std::string& name, bool isWhite, bool isActive,
                                   const std::vector<char>& capturedPieces, int materialAdvantage) {
    int cardX = pImpl->sidebarX + 16;
    int cardY = pImpl->sidebarY + yOffset;
    int cardW = pImpl->sidebarWidth - 32;
    int cardH = 58;

    Rectangle cardRect = {
        static_cast<float>(cardX),
        static_cast<float>(cardY),
        static_cast<float>(cardW),
        static_cast<float>(cardH)
    };

    Color bgCol = isActive ? CLITERAL(Color){ 34, 40, 52, 255 } : CLITERAL(Color){ 26, 30, 38, 255 };
    Color borderCol = isActive ? CLITERAL(Color){ 80, 120, 180, 255 } : CLITERAL(Color){ 38, 44, 56, 255 };

    DrawRectangleRounded(cardRect, 0.15f, 4, bgCol);
    DrawRectangleRoundedLinesEx(cardRect, 0.15f, 4, 1.0f, borderCol);

    // Player color badge
    int avatarX = cardX + 18;
    int avatarY = cardY + 29;
    Color avatarCol = isWhite ? CLITERAL(Color){ 245, 245, 250, 255 } : CLITERAL(Color){ 30, 34, 42, 255 };
    DrawCircle(avatarX, avatarY, 11.0f, avatarCol);
    DrawCircleLines(avatarX, avatarY, 11.0f, CLITERAL(Color){ 160, 170, 185, 255 });

    // Player Name
    DrawText(name.c_str(), cardX + 38, cardY + 12, 15, CLITERAL(Color){ 240, 244, 252, 255 });

    // Turn indicator pill
    if (isActive) {
        Rectangle turnPill = {
            static_cast<float>(cardX + cardW - 74),
            static_cast<float>(cardY + 10),
            62.0f,
            18.0f
        };
        DrawRectangleRounded(turnPill, 0.4f, 4, CLITERAL(Color){ 30, 80, 50, 255 });
        DrawRectangleRoundedLinesEx(turnPill, 0.4f, 4, 1.0f, CLITERAL(Color){ 50, 180, 90, 255 });
        DrawText("TO MOVE", static_cast<int>(turnPill.x + 8), static_cast<int>(turnPill.y + 3), 10, CLITERAL(Color){ 130, 240, 160, 255 });
    }

    // Material advantage & captured pieces list
    int rackX = cardX + 38;
    int rackY = cardY + 34;
    std::string rackStr;
    for (size_t i = 0; i < capturedPieces.size() && i < 10; ++i) {
        rackStr += capturedPieces[i];
        rackStr += " ";
    }
    if (materialAdvantage > 0) {
        rackStr += "+" + std::to_string(materialAdvantage);
    }
    DrawText(rackStr.c_str(), rackX, rackY, 12, CLITERAL(Color){ 160, 175, 195, 255 });
}

void RaylibRenderer::drawStatusBanner(const std::string& statusText, int statusType) {
    int bannerX = pImpl->sidebarX + 16;
    int bannerY = pImpl->sidebarY + 162;
    int bannerW = pImpl->sidebarWidth - 32;
    int bannerH = 34;

    Rectangle bannerRect = {
        static_cast<float>(bannerX),
        static_cast<float>(bannerY),
        static_cast<float>(bannerW),
        static_cast<float>(bannerH)
    };

    Color bgCol = CLITERAL(Color){ 30, 36, 46, 255 };
    Color borderCol = CLITERAL(Color){ 50, 60, 78, 255 };
    Color textCol = CLITERAL(Color){ 220, 230, 245, 255 };

    if (statusType == 1) { // Check
        bgCol = CLITERAL(Color){ 64, 28, 28, 255 };
        borderCol = CLITERAL(Color){ 190, 60, 60, 255 };
        textCol = CLITERAL(Color){ 255, 180, 180, 255 };
    } else if (statusType == 2) { // Checkmate
        bgCol = CLITERAL(Color){ 24, 56, 36, 255 };
        borderCol = CLITERAL(Color){ 50, 180, 90, 255 };
        textCol = CLITERAL(Color){ 170, 255, 190, 255 };
    } else if (statusType == 3) { // Stalemate / Draw
        bgCol = CLITERAL(Color){ 44, 46, 54, 255 };
        borderCol = CLITERAL(Color){ 120, 130, 145, 255 };
        textCol = CLITERAL(Color){ 210, 215, 225, 255 };
    }

    DrawRectangleRounded(bannerRect, 0.2f, 4, bgCol);
    DrawRectangleRoundedLinesEx(bannerRect, 0.2f, 4, 1.0f, borderCol);

    int textW = MeasureText(statusText.c_str(), 14);
    int textX = bannerX + (bannerW - textW) / 2;
    int textY = bannerY + 10;
    DrawText(statusText.c_str(), textX, textY, 14, textCol);
}

void RaylibRenderer::drawMoveHistory(const std::vector<std::string>& whiteMoves,
                                    const std::vector<std::string>& blackMoves,
                                    int& scrollOffset) {
    int boxX = pImpl->sidebarX + 16;
    int boxY = pImpl->sidebarY + 208;
    int boxW = pImpl->sidebarWidth - 32;
    int boxH = pImpl->sidebarHeight - 340;

    Rectangle boxRect = {
        static_cast<float>(boxX),
        static_cast<float>(boxY),
        static_cast<float>(boxW),
        static_cast<float>(boxH)
    };

    DrawRectangleRounded(boxRect, 0.08f, 4, CLITERAL(Color){ 20, 23, 30, 255 });
    DrawRectangleRoundedLinesEx(boxRect, 0.08f, 4, 1.0f, CLITERAL(Color){ 36, 42, 54, 255 });

    // Header bar
    Rectangle headerRect = { static_cast<float>(boxX), static_cast<float>(boxY), static_cast<float>(boxW), 26.0f };
    DrawRectangleRounded(headerRect, 0.08f, 4, CLITERAL(Color){ 28, 32, 42, 255 });
    DrawText("#", boxX + 12, boxY + 7, 12, CLITERAL(Color){ 140, 150, 170, 255 });
    DrawText("WHITE", boxX + 60, boxY + 7, 12, CLITERAL(Color){ 140, 150, 170, 255 });
    DrawText("BLACK", boxX + 160, boxY + 7, 12, CLITERAL(Color){ 140, 150, 170, 255 });

    // Handle mouse wheel scrolling
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, boxRect)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            scrollOffset -= static_cast<int>(wheel * 2);
        }
    }

    size_t numTurns = std::max(whiteMoves.size(), blackMoves.size());
    int visibleRows = (boxH - 30) / 22;
    int maxScroll = std::max(0, static_cast<int>(numTurns) - visibleRows);
    scrollOffset = std::clamp(scrollOffset, 0, maxScroll);

    // Scissor mode for clean clipping
    BeginScissorMode(boxX + 2, boxY + 28, boxW - 4, boxH - 32);

    int rowY = boxY + 30;
    for (size_t i = static_cast<size_t>(scrollOffset); i < numTurns; ++i) {
        if (rowY + 20 > boxY + boxH) break;

        bool isEvenRow = (i % 2 == 0);
        if (isEvenRow) {
            DrawRectangle(boxX + 4, rowY, boxW - 8, 20, CLITERAL(Color){ 24, 28, 36, 160 });
        }

        std::string numStr = std::to_string(i + 1) + ".";
        DrawText(numStr.c_str(), boxX + 12, rowY + 4, 12, CLITERAL(Color){ 110, 120, 140, 255 });

        if (i < whiteMoves.size()) {
            DrawText(whiteMoves[i].c_str(), boxX + 60, rowY + 4, 12, CLITERAL(Color){ 230, 235, 245, 255 });
        }
        if (i < blackMoves.size()) {
            DrawText(blackMoves[i].c_str(), boxX + 160, rowY + 4, 12, CLITERAL(Color){ 190, 200, 215, 255 });
        }

        rowY += 22;
    }

    EndScissorMode();
}

UIAction RaylibRenderer::drawSidebarControls(bool canUndo) {
    int btnAreaX = pImpl->sidebarX + 16;
    int btnAreaY = pImpl->sidebarY + pImpl->sidebarHeight - 110;
    int btnAreaW = pImpl->sidebarWidth - 32;

    int halfW = (btnAreaW - 8) / 2;
    int thirdW = (btnAreaW - 16) / 3;

    // Row 1: New Game and Restart
    Rectangle btnNew = { static_cast<float>(btnAreaX), static_cast<float>(btnAreaY), static_cast<float>(halfW), 32.0f };
    Rectangle btnRestart = { static_cast<float>(btnAreaX + halfW + 8), static_cast<float>(btnAreaY), static_cast<float>(halfW), 32.0f };

    // Row 2: Undo, Flip, Settings
    int row2Y = btnAreaY + 40;
    Rectangle btnUndo = { static_cast<float>(btnAreaX), static_cast<float>(row2Y), static_cast<float>(thirdW), 30.0f };
    Rectangle btnFlip = { static_cast<float>(btnAreaX + thirdW + 8), static_cast<float>(row2Y), static_cast<float>(thirdW), 30.0f };
    Rectangle btnSettings = { static_cast<float>(btnAreaX + 2 * (thirdW + 8)), static_cast<float>(row2Y), static_cast<float>(thirdW), 30.0f };

    if (pImpl->drawButton(btnNew, "New Game", false)) return UIAction::NewGame;
    if (pImpl->drawButton(btnRestart, "Restart", false)) return UIAction::Restart;
    if (pImpl->drawButton(btnUndo, "Undo", !canUndo)) return UIAction::Undo;
    if (pImpl->drawButton(btnFlip, "Flip", false)) return UIAction::FlipBoard;
    if (pImpl->drawButton(btnSettings, "Settings", false)) return UIAction::OpenSettings;

    return UIAction::None;
}

char RaylibRenderer::drawPromotionModal(bool isWhite) {
    // Backdrop blur / dim
    DrawRectangle(0, 0, pImpl->windowWidth, pImpl->windowHeight, CLITERAL(Color){ 10, 12, 16, 200 });

    int modalW = 380;
    int modalH = 190;
    int modalX = (pImpl->windowWidth - modalW) / 2;
    int modalY = (pImpl->windowHeight - modalH) / 2;

    Rectangle modalRect = { static_cast<float>(modalX), static_cast<float>(modalY), static_cast<float>(modalW), static_cast<float>(modalH) };
    DrawRectangleRounded(modalRect, 0.12f, 4, CLITERAL(Color){ 28, 33, 44, 255 });
    DrawRectangleRoundedLinesEx(modalRect, 0.12f, 4, 2.0f, CLITERAL(Color){ 70, 90, 120, 255 });

    DrawText("PAWN PROMOTION", modalX + 24, modalY + 20, 18, CLITERAL(Color){ 245, 248, 255, 255 });
    DrawText("Select piece to promote your pawn to:", modalX + 24, modalY + 46, 13, CLITERAL(Color){ 160, 175, 195, 255 });

    char pieces[] = { 'Q', 'R', 'B', 'N' };
    const char* names[] = { "Queen", "Rook", "Bishop", "Knight" };

    int tileW = 72;
    int tileH = 76;
    int gap = 12;
    int startX = modalX + (modalW - (4 * tileW + 3 * gap)) / 2;
    int startY = modalY + 84;

    Vector2 mouse = GetMousePosition();
    char selected = '\0';

    for (int i = 0; i < 4; ++i) {
        Rectangle tile = { static_cast<float>(startX + i * (tileW + gap)), static_cast<float>(startY), static_cast<float>(tileW), static_cast<float>(tileH) };
        bool hovered = CheckCollisionPointRec(mouse, tile);
        bool clicked = hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

        Color tileBg = hovered ? CLITERAL(Color){ 46, 56, 75, 255 } : CLITERAL(Color){ 36, 42, 54, 255 };
        Color tileBorder = hovered ? CLITERAL(Color){ 120, 160, 220, 255 } : CLITERAL(Color){ 55, 65, 82, 255 };

        DrawRectangleRounded(tile, 0.18f, 4, tileBg);
        DrawRectangleRoundedLinesEx(tile, 0.18f, 4, 1.5f, tileBorder);

        // Piece icon
        float cx = tile.x + tile.width / 2.0f;
        float cy = tile.y + 30.0f;
        pImpl->drawProceduralPiece(cx, cy, 60.0f, pieces[i], isWhite);

        // Piece name
        int nameW = MeasureText(names[i], 12);
        DrawText(names[i], static_cast<int>(cx - nameW / 2.0f), static_cast<int>(tile.y + 56), 12, CLITERAL(Color){ 220, 230, 245, 255 });

        if (clicked) {
            selected = pieces[i];
        }
    }

    return selected;
}

ModalResult RaylibRenderer::drawConfirmationModal(const std::string& title, const std::string& message) {
    DrawRectangle(0, 0, pImpl->windowWidth, pImpl->windowHeight, CLITERAL(Color){ 10, 12, 16, 200 });

    int modalW = 420;
    int modalH = 190;
    int modalX = (pImpl->windowWidth - modalW) / 2;
    int modalY = (pImpl->windowHeight - modalH) / 2;

    Rectangle modalRect = { static_cast<float>(modalX), static_cast<float>(modalY), static_cast<float>(modalW), static_cast<float>(modalH) };
    DrawRectangleRounded(modalRect, 0.12f, 4, CLITERAL(Color){ 28, 33, 44, 255 });
    DrawRectangleRoundedLinesEx(modalRect, 0.12f, 4, 2.0f, CLITERAL(Color){ 70, 90, 120, 255 });

    DrawText(title.c_str(), modalX + 28, modalY + 24, 18, CLITERAL(Color){ 245, 248, 255, 255 });
    DrawText(message.c_str(), modalX + 28, modalY + 56, 13, CLITERAL(Color){ 170, 180, 200, 255 });

    Rectangle btnConfirm = { static_cast<float>(modalX + modalW - 230), static_cast<float>(modalY + modalH - 52), 100.0f, 32.0f };
    Rectangle btnCancel = { static_cast<float>(modalX + modalW - 120), static_cast<float>(modalY + modalH - 52), 92.0f, 32.0f };

    if (pImpl->drawButton(btnConfirm, "Confirm", false)) return ModalResult::Confirm;
    if (pImpl->drawButton(btnCancel, "Cancel", false)) return ModalResult::Cancel;

    return ModalResult::Pending;
}

bool RaylibRenderer::drawSettingsModal(UISettings& settings) {
    DrawRectangle(0, 0, pImpl->windowWidth, pImpl->windowHeight, CLITERAL(Color){ 10, 12, 16, 200 });

    int modalW = 460;
    int modalH = 340;
    int modalX = (pImpl->windowWidth - modalW) / 2;
    int modalY = (pImpl->windowHeight - modalH) / 2;

    Rectangle modalRect = { static_cast<float>(modalX), static_cast<float>(modalY), static_cast<float>(modalW), static_cast<float>(modalH) };
    DrawRectangleRounded(modalRect, 0.12f, 4, CLITERAL(Color){ 26, 31, 42, 255 });
    DrawRectangleRoundedLinesEx(modalRect, 0.12f, 4, 2.0f, CLITERAL(Color){ 65, 82, 110, 255 });

    DrawText("SETTINGS", modalX + 28, modalY + 22, 18, CLITERAL(Color){ 245, 248, 255, 255 });

    // Section 1: Themes
    DrawText("BOARD THEME", modalX + 28, modalY + 62, 12, CLITERAL(Color){ 130, 145, 170, 255 });

    int themeW = 92;
    int themeH = 30;
    int themeY = modalY + 84;

    Rectangle t1 = { static_cast<float>(modalX + 28), static_cast<float>(themeY), static_cast<float>(themeW), static_cast<float>(themeH) };
    Rectangle t2 = { static_cast<float>(modalX + 28 + themeW + 10), static_cast<float>(themeY), static_cast<float>(themeW), static_cast<float>(themeH) };
    Rectangle t3 = { static_cast<float>(modalX + 28 + 2 * (themeW + 10)), static_cast<float>(themeY), static_cast<float>(themeW), static_cast<float>(themeH) };
    Rectangle t4 = { static_cast<float>(modalX + 28 + 3 * (themeW + 10)), static_cast<float>(themeY), static_cast<float>(themeW), static_cast<float>(themeH) };

    if (pImpl->drawButton(t1, "Walnut", false, settings.theme == BoardTheme::ClassicWood)) settings.theme = BoardTheme::ClassicWood;
    if (pImpl->drawButton(t2, "Slate", false, settings.theme == BoardTheme::ModernSlate)) settings.theme = BoardTheme::ModernSlate;
    if (pImpl->drawButton(t3, "Emerald", false, settings.theme == BoardTheme::ForestGreen)) settings.theme = BoardTheme::ForestGreen;
    if (pImpl->drawButton(t4, "Midnight", false, settings.theme == BoardTheme::MidnightBlue)) settings.theme = BoardTheme::MidnightBlue;

    // Section 2: Display Options
    DrawText("DISPLAY PREFERENCES", modalX + 28, modalY + 138, 12, CLITERAL(Color){ 130, 145, 170, 255 });

    Rectangle opt1 = { static_cast<float>(modalX + 28), static_cast<float>(modalY + 160), 396.0f, 32.0f };
    Rectangle opt2 = { static_cast<float>(modalX + 28), static_cast<float>(modalY + 200), 396.0f, 32.0f };
    Rectangle opt3 = { static_cast<float>(modalX + 28), static_cast<float>(modalY + 240), 396.0f, 32.0f };

    std::string textOpt1 = std::string("Board Coordinates:  ") + (settings.showCoordinates ? "[ON]" : "[OFF]");
    std::string textOpt2 = std::string("Legal Move Hints:   ") + (settings.showMoveHints ? "[ON]" : "[OFF]");
    std::string textOpt3 = std::string("Highlight Last Move: ") + (settings.highlightLastMove ? "[ON]" : "[OFF]");

    if (pImpl->drawButton(opt1, textOpt1.c_str(), false, settings.showCoordinates)) {
        settings.showCoordinates = !settings.showCoordinates;
    }
    if (pImpl->drawButton(opt2, textOpt2.c_str(), false, settings.showMoveHints)) {
        settings.showMoveHints = !settings.showMoveHints;
    }
    if (pImpl->drawButton(opt3, textOpt3.c_str(), false, settings.highlightLastMove)) {
        settings.highlightLastMove = !settings.highlightLastMove;
    }

    // Done button
    Rectangle btnDone = { static_cast<float>(modalX + modalW - 128), static_cast<float>(modalY + modalH - 46), 100.0f, 30.0f };
    if (pImpl->drawButton(btnDone, "Done", false)) {
        return true;
    }

    return false;
}

void RaylibRenderer::drawToast(const std::string& message, float opacity) {
    if (message.empty() || opacity <= 0.01f) return;

    int textW = MeasureText(message.c_str(), 14);
    int toastW = textW + 36;
    int toastH = 34;

    int boardCenterX = pImpl->boardOffsetX + 4 * pImpl->squareSize;
    int toastX = boardCenterX - toastW / 2;
    int toastY = pImpl->boardOffsetY + 8 * pImpl->squareSize - 44;

    unsigned char alpha = static_cast<unsigned char>(std::clamp(opacity, 0.0f, 1.0f) * 230);

    Rectangle toastRect = { static_cast<float>(toastX), static_cast<float>(toastY), static_cast<float>(toastW), static_cast<float>(toastH) };
    DrawRectangleRounded(toastRect, 0.4f, 4, CLITERAL(Color){ 18, 22, 28, alpha });
    DrawRectangleRoundedLinesEx(toastRect, 0.4f, 4, 1.2f, CLITERAL(Color){ 80, 95, 125, alpha });

    DrawText(message.c_str(), toastX + 18, toastY + 10, 14, CLITERAL(Color){ 245, 210, 120, alpha });
}

void RaylibRenderer::drawUI(const std::string& turnText, bool isWhiteTurn, const std::string& statusMessage) {
    (void)isWhiteTurn;
    drawSidebarHeader();
    drawStatusBanner(turnText, 0);
    if (!statusMessage.empty()) {
        drawToast(statusMessage, 0.9f);
    }
}

bool RaylibRenderer::getMouseBoardSquare(int& outRow, int& outCol, bool flipped) const {
    Vector2 m = GetMousePosition();
    int sq = pImpl->squareSize;

    if (m.x >= pImpl->boardOffsetX &&
        m.y >= pImpl->boardOffsetY &&
        m.x < pImpl->boardOffsetX + 8 * sq &&
        m.y < pImpl->boardOffsetY + 8 * sq) {

        int dispC = static_cast<int>((m.x - pImpl->boardOffsetX) / sq);
        int dispR = static_cast<int>((m.y - pImpl->boardOffsetY) / sq);

        dispC = std::clamp(dispC, 0, 7);
        dispR = std::clamp(dispR, 0, 7);

        outCol = flipped ? (7 - dispC) : dispC;
        outRow = flipped ? (7 - dispR) : dispR;
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

bool RaylibRenderer::isKeyFPressed() const {
    return IsKeyPressed(KEY_F);
}

bool RaylibRenderer::isKeyUPressed() const {
    return IsKeyPressed(KEY_U);
}

bool RaylibRenderer::isKeyEscapePressed() const {
    return IsKeyPressed(KEY_ESCAPE);
}

float RaylibRenderer::getMouseWheelMove() const {
    return GetMouseWheelMove();
}
