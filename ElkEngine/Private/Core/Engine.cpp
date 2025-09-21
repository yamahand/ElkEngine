#include "ElkEngine/Core/Engine.h"
#include "ElkEngine/Core/Logger/GameLogger.h"

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <atomic>

namespace elk
{
    struct EngineImpl {
        std::atomic<bool> running{false};
        std::chrono::steady_clock::time_point lastTick;
    };

    // 定義: static メンバ
    Engine* Engine::s_instance = nullptr;

    Engine::Engine()
    {
        m_impl = new EngineImpl();
    }

    Engine::~Engine()
    {
        delete m_impl;
        m_impl = nullptr;
        if (s_instance == this) s_instance = nullptr;
    }

    bool Engine::Initialize()
    {
        // 最低限の初期化を行い成功を返す
        if (!m_impl) m_impl = new EngineImpl();
        s_instance = this;
        m_impl->lastTick = std::chrono::steady_clock::now();
        std::cout << "Engine initialized\n";
        DefaultLogger::Initialize("logs/game.log");
		int tmp = 42; // ダミー変数
		GAME_LOG_DEBUG("Engine", "Engine logger initialized {0}", tmp);
        return true;
    }

    void Engine::Run(Application* application)
    {
        if (!application) {
            std::cerr << "No application provided to Engine::Run\n";
            return;
        }

        // アプリケーション初期化
        if (!application->Initialize()) {
            std::cerr << "Application::Initialize failed\n";
            return;
        }

        m_impl->running = true;
        m_impl->lastTick = std::chrono::steady_clock::now();

        // シンプルなメインループ（固定タイムステップ/簡易版）
        while (application->IsRunning()) {
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<float> delta = now - m_impl->lastTick;
            m_impl->lastTick = now;

            application->Update(delta.count());
            application->Render();

            // 最小スリープでCPUを休める（1ms）
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        application->Shutdown();
        m_impl->running = false;
    }

    void Engine::Shutdown()
    {
        if (m_impl) {
            m_impl->running = false;
        }
        std::cout << "Engine shutdown\n";
    }

    ENGINE_API Engine* CreateEngine()
    {
        return new Engine();
    }

    ENGINE_API void DestroyEngine(Engine* engine)
    {
        delete engine;
    }
}