#include "BoardView.hpp"
#include "ChessEngine.hpp"

int main() {
    // 1. Initialize desktop window (800x800) and raylib graphics context
    BoardView view(800, 800);
    view.init();

    // 2. Initialize core chess engine state
    ChessEngine engine;

    // 3. Main interactive application frame loop
    while (!view.shouldClose()) {
        view.processInput(engine);
        view.render(engine);
    }

    // 4. Clean up resources and close window
    view.cleanup();
    return 0;
}
