#pragma once

#include "Application.h"

namespace elk {

class ENGINE_API Engine {
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run(Application* application);
    void Shutdown();

    static Engine* GetInstance() { return s_instance; }

private:
    static Engine* s_instance;
    struct EngineImpl* m_impl = nullptr;
};

// Factory functions
ENGINE_API Engine* CreateEngine();
ENGINE_API void DestroyEngine(Engine* engine);

} // namespace elk
