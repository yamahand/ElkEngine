#pragma once

#include "ElkEngine/Core/Application.h"

class GameApplication : public ElkEngine::Application {
public:
    GameApplication() = default;
    ~GameApplication() override = default;

    bool Initialize() override;
    void Update(float deltaTime) override;
    void Render() override;
    void Shutdown() override;

    const char* GetName() const override { return "ElkGame"; }

private:
    // Game-specific members
};
