#include <SFML/Graphics.hpp>

int main() {
    // 1. Initialize the render window with 800x600 resolution and title 'KnightForge'
    sf::RenderWindow window(sf::VideoMode(800, 600), "KnightForge");
    window.setFramerateLimit(60);

    // 2. Main application event and render loop
    while (window.isOpen()) {
        // Process pending OS window events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Clear the frame with a clean dark background color
        window.clear(sf::Color(28, 28, 30));

        // Present rendered frame buffer
        window.display();
    }

    return 0;
}
