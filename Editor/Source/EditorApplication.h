#pragma once

class EditorApplication {
public:
    EditorApplication() = default;
    ~EditorApplication() = default;

    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool m_running = false;
    
    void ProcessEvents();
    void Update();
    void Render();
};
