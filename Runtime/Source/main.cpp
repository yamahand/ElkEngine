#include "ElkEngine/Core/Engine.h"
#include "GameApplication.h"

#include <memory>

int main() {
    // Create engine
    auto* engine = elk::CreateEngine();
    if (!engine) {
        return -1;
    }

    // Create game application
    auto game = std::make_unique<GameApplication>();

    // Run the engine
    engine->Run(game.get());

    // Cleanup
    elk::DestroyEngine(engine);
    return 0;
}
