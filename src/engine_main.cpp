#include "GameEngine.h"
#include <iostream>

int main() {
    // Initialize game engine
    const unsigned int SCREEN_WIDTH = 1920;
    const unsigned int SCREEN_HEIGHT = 1080;
    const char* WINDOW_TITLE = "Surgical Simulation System";
    
    GameEngine engine(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    
    // Initialize engine
    if (!engine.initialize()) {
        std::cout << "Failed to initialize game engine!" << std::endl;
        return -1;
    }
    
    std::cout << "Starting Surgical Simulation System..." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "- Menu: Use 1/2/3 keys to select options, Enter to confirm" << std::endl;
    std::cout << "- Game: ESC to return to menu, P to pause" << std::endl;
    std::cout << "- Use keyboard controls for forceps if haptic device is not available" << std::endl;
    
    // Run game loop
    engine.run();
    
    // Shutdown
    engine.shutdown();
    
    std::cout << "System shutdown complete." << std::endl;
    return 0;
}
