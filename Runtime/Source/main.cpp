#include "ElkEngine/Core/Engine.h"
#include "GameApplication.h"

int main() {
    // Create engine
    auto* engine = ElkEngine::CreateEngine();
    if (!engine) {
        return -1;
    }

    // Create game application
    auto game = std::make_unique<GameApplication>();

    // Run the engine
    engine->Run(game.get());

    // Cleanup
    ElkEngine::DestroyEngine(engine);
    return 0;
}
