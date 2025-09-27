#include "Core/Engine.h"
#include "Core/Logger/Logger.h"
#include "Core/Logger/TagRegistry.h"
#include "Core/Logger/LoggerService.h"
#include "Core/Utility/ServiceLocator.h"

#include <chrono>
#include <thread>
#include <memory>
#include <iostream>
#include <atomic>

namespace elk
{
	struct EngineImpl
	{
		std::atomic<bool> running{ false };
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
		if (s_instance == this)
			s_instance = nullptr;
	}

	bool Engine::Initialize()
	{
		// 最低限の初期化を行い成功を返す
		if (!m_impl)
			m_impl = new EngineImpl();
		s_instance = this;
		m_impl->lastTick = std::chrono::steady_clock::now();
		std::cout << "Engine initialized\n";
		InitializeServices();
		// ロガーは InitializeServices() 内で登録・初期化済みなのでマクロ経由でログ出力
		ELK_LOG_TRACE("Engine", "GAME_LOG_TRACE");
		ELK_LOG_DEBUG("Engine", "GAME_LOG_DEBUG");
		ELK_LOG_INFO("Engine", "GAME_LOG_INFO");
		ELK_LOG_WARN("Engine", "GAME_LOG_WARN");
		ELK_LOG_ERROR("Engine", "GAME_LOG_ERROR");
		ELK_LOG_CRITICAL("Engine", "GAME_LOG_CRITICAL");
		return true;
	}

	void Engine::Run(Application* application)
	{
		if (!application)
		{
			std::cerr << "No application provided to Engine::Run\n";
			return;
		}

		// アプリケーション初期化
		if (!application->Initialize())
		{
			std::cerr << "Application::Initialize failed\n";
			return;
		}

		m_impl->running = true;
		m_impl->lastTick = std::chrono::steady_clock::now();

		// シンプルなメインループ（固定タイムステップ/簡易版）
		while (application->IsRunning())
		{
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
		if (m_impl)
		{
			m_impl->running = false;
		}
		std::cout << "Engine shutdown\n";
	}

	void Engine::InitializeServices()
	{
		ServiceLocator::Register<logger::TagRegistry>(std::make_shared<logger::TagRegistry>());
		// LoggerService を登録 (バックエンドが有効なときのみ)
		ServiceLocator::Register<DefaultLoggerService>(std::make_shared<DefaultLoggerService>());

		// 初期化とログ出力は ServiceLocator 経由で行う
		if (auto logger = LOGGER_SERVICE()) {
			logger->Initialize("logs/engine.log");
			logger->SetLogLevel(LogLevel::Trace);
		}
	}

	void Engine::ShutdownServices()
	{
		//ServiceLocator::Unregister<Logger>();
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