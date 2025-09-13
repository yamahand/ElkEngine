#include "GameApplication.h"
#include <iostream>

bool GameApplication::Initialize() {
    std::cout << "Game initialized!" << std::endl;
    return true;
}

void GameApplication::Update(float deltaTime) {
    // Update game logic
    (void)deltaTime; // Suppress unused parameter warning
}

void GameApplication::Render() {
    // Render game
}

void GameApplication::Shutdown() {
    std::cout << "Game shutdown!" << std::endl;
}
