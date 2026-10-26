#include "BoardView.hpp"
#include "EngineAdapter.hpp"

int main() {
    // 1. Initialize core chess engine adapter conforming to IChessGame interface
    auto game = std::make_shared<EngineAdapter>();

    // 2. Initialize desktop window (1100x750, resizable) and raylib presentation view
    BoardView view(game, 1100, 750);
    view.init();

    // 3. Main interactive application frame loop
    while (!view.shouldClose()) {
        view.processInput();
        view.render();
    }

    // 4. Clean up resources and close window
    view.cleanup();
    return 0;
}
