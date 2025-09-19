#pragma once

#include "EngineAPI.h"

namespace elk {

class ENGINE_API Application {
public:
    Application() = default;
    virtual ~Application() = default;

    // アプリケーションライフサイクル
    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;

    // アプリケーション情報
    virtual const char* GetName() const = 0;
    virtual const char* GetVersion() const { return "1.0.0"; }

    // オプションのイベントハンドラ
    virtual void OnWindowResize(int width, int height) { ELK_UNUSED(width); ELK_UNUSED(height); }
    virtual void OnWindowClose() {}
    
    // アプリケーション状態
    bool IsRunning() const { return m_running; }
    
protected:
    void RequestExit() { m_running = false; }
    
private:
    bool m_running = true;
    
    friend class Engine; // Engine クラスからアクセス可能
};

} // namespace elk