#pragma once

#include <string>

/**
 * @brief Available chessboard visual themes.
 */
enum class BoardTheme {
    ClassicWood,  // Warm parchment and walnut tones
    ModernSlate,  // Contemporary silver and slate blue
    ForestGreen,  // Traditional club green and ivory
    MidnightBlue  // Elegant deep indigo and cool grey
};

/**
 * @brief User-configurable UI and board preferences.
 */
struct UISettings {
    BoardTheme theme{BoardTheme::ClassicWood};
    bool showCoordinates{true};
    bool showMoveHints{true};
    bool highlightLastMove{true};
    bool flipped{false};
};
