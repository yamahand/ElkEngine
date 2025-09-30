# AGENTS.md - ElkGameEngine AI開発支援ドキュメント

このドキュメントは、AI開発者（Claude、GitHub Copilot、ChatGPT等）がElkGameEngineプロジェクトの開発を効率的に支援するためのガイドラインです。

## 🎯 プロジェクト概要

**ElkGameEngine**は、c++23とCMakeを使用したモダンなクロスプラットフォーム対応ゲームエンジンです。

### 基本情報
- **言語**: C++23（コア）+ C# WPF（エディタ）
- **ビルドシステム**: CMake 4.0+
- **対象プラットフォーム**: Windows 11, Linux, macOS
- **グラフィックスAPI**: DirectX 12, Vulkan（計画中）
- **アーキテクチャ**: ブリッジ統合型（C++コア + C#エディタ）
- **エラーハンドリング**: std::expected中心（例外最小限）

## 📁 プロジェクト構造の理解

```
ElkGameEngine/
├── Assets/                 # ゲーム開発用アセット
├── Documentation/          # プロジェクトドキュメント
│   └── 実装仕様書.md      # 詳細な技術仕様
├── Editor/                 # C# WPFエディタ（計画中）
├── EditorBridge/          # C++からC#への橋渡しライブラリ
├── ElkEngine/             # エンジンのコアライブラリ
│   ├── Public/            # 公開APIヘッダ
│   └── Private/           # 内部実装
├── Runtime/               # ランタイムアプリケーション
├── Scripts/               # ビルド・セットアップスクリプト
├── ThirdParty/           # 外部ライブラリ管理
└── cmake/                # CMakeヘルパー関数
```

## 🔧 開発時の重要なガイドライン

### コーディング規約

#### 命名規則
```cpp
// 名前空間: 小文字
namespace elk::rendering { }

// クラス: PascalCase
class RenderSystem { };

// 関数: PascalCase
void Initialize();

// 変数: camelCase
float deltaTime;

// メンバ変数: m_camelCase
bool m_isRunning;

// 定数: UPPER_SNAKE_CASE
constexpr int MAX_ENTITIES = 1000;
```

#### c++23機能の積極的な活用
```cpp
// コンセプトを使用した型制約
template<typename T>
concept Component = requires(T t) {
    { t.GetTypeId() } -> std::convertible_to<size_t>;
};

// std::expected を使用したエラーハンドリング
std::expected<Pipeline*, Error> CreatePipeline(const PipelineDesc& desc);

// Ranges ライブラリの活用
auto activeEntities = entities 
    | std::views::filter([](const auto& e) { return e.IsActive(); })
    | std::views::transform([](const auto& e) { return e.GetId(); });
```

### APIの設計原則

1. **RAII**: リソース管理は必ずRAIIパターンを使用
2. **const-correctness**: 可能な限りconstを使用
3. **[[nodiscard]]**: 戻り値を無視してはいけない関数に必須
4. **noexcept**: 例外を投げない関数には必須

```cpp
class ENGINE_API Engine final {
public:
    [[nodiscard]] bool Initialize(const EngineConfig& config) noexcept;
    [[nodiscard]] static Engine& GetInstance() noexcept;
    
    // 削除されたコピー/ムーブ
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
};
```

## 🏗️ システムアーキテクチャ

### エンジンコア（ElkEngine）
- **Core**: アプリケーションフレームワーク、エンジン管理
- **Renderer**: グラフィックス抽象化レイヤー（RHI）
- **ECS**: Entity Component System
- **Logger**: 構造化ログシステム（spdlog使用）
- **Math**: 数学ユーティリティ

### エディタブリッジ（EditorBridge）
```cpp
// C APIでエクスポート（P/Invoke用）
extern "C" {
    BRIDGE_API void* CreateEditorEngine();
    BRIDGE_API bool InitializeEngine(void* engine, const char* configJson);
    BRIDGE_API void* CreateViewport(void* engine, intptr_t hwnd, int width, int height);
}
```

### 型登録システム（TypeRegistry）
```cpp
// constexpr ハッシュによる型登録
static const uint32_t TRANSFORM_TYPE_ID = ELK_REGISTER_TYPE("elk::ecs::Transform");

// 実行時登録
uint32_t id = TypeRegistry::Instance().Register(hash, "MyComponent");
```

## 🔄 開発ワークフロー

### 新機能追加時の手順

1. **ヘッダ作成**: `ElkEngine/Public/ElkEngine/[Module]/` に公開API
2. **実装追加**: `ElkEngine/Private/[Module]/` に実装
3. **CMake更新**: `ElkEngine/CMakeLists.txt` にソース追加
4. **テスト作成**: `Runtime/Source/` でテスト実装
5. **ドキュメント更新**: このAGENTS.mdや実装仕様書.mdを更新

### ビルド方法
```bash
# Windows
Scripts\build.bat clean release

# Linux/macOS  
./Scripts/build.sh clean release
```

### CMakeターゲット構成
- **ElkEngine**: コアエンジンライブラリ
- **EditorBridge**: C++/C#ブリッジDLL
- **Runtime**: ゲームランタイム実行ファイル
- **Editor**: C# WPFエディタ（計画中）

## 📋 よくある開発タスク

### 新しいコンポーネントの追加
```cpp
// 1. Public ヘッダで定義
// ElkEngine/Public/ElkEngine/ECS/Components/MyComponent.h
namespace elk::ecs {
    struct MyComponent {
        static constexpr uint32_t TYPE_ID = ELK_REGISTER_TYPE("elk::ecs::MyComponent");
        float value;
    };
}

// 2. 必要に応じて実装ファイル作成
// ElkEngine/Private/ECS/Components/MyComponent.cpp
```

### レンダリング機能の追加
```cpp
// 1. RHI抽象化に新機能追加
// ElkEngine/Public/ElkEngine/Renderer/RHI.h
class RHI {
public:
    virtual std::expected<NewResource*, Error> CreateNewResource() = 0;
};

// 2. 各実装で対応
// ElkEngine/Private/Renderer/DirectX12/D3D12RHI.cpp
// ElkEngine/Private/Renderer/Vulkan/VulkanRHI.cpp
```

### ログ出力の追加
```cpp
// 構造化ログの使用
GAME_LOG_INFO("Engine", "エンジンが初期化されました {}", version);
GAME_LOG_ERROR("Renderer", "描画エラー: {}", errorCode);

// カテゴリ付きログ
ELK_LOG_DEBUG("Memory", "Allocator", "メモリ割り当て: {} bytes", size);
```

## 🚨 注意すべきポイント

### メモリ管理
- **スマートポインタ優先**: `std::unique_ptr`, `std::shared_ptr`を積極的に使用
- **RAII徹底**: リソースの取得と解放は必ずRAIIで管理
- **メモリリーク検出**: デバッグビルドではメモリリーク検出を有効化

### エラーハンドリング方針

#### 🎯 C++23 std::expected 中心のアプローチ

**基本方針**: 例外は使用せず、`std::expected`でエラーハンドリングを統一

```cpp
// ElkEngine標準のエラーハンドリング
#include <expected>

template<typename T, typename E>
using Result = std::expected<T, E>;

// 使用例
Result<Model, LoadError> LoadModel(const std::string& path) {
    if (!fs::exists(path)) {
        return std::unexpected(LoadError::FileNotFound);
    }
    return model;
}

// チェーン可能な処理
auto pipeline = LoadModel("asset.obj")
    .and_then([](auto&& model) { return ValidateModel(model); })
    .and_then([](auto&& model) { return OptimizeModel(model); });
```

#### 例外使用ガイドライン
- **禁止**: ゲームループ内、リアルタイム処理
- **許可**: エンジン初期化時のみ（一度だけの処理）
- **推奨**: すべて`std::expected`で統一

2. **初期化時のみ**: 例外使用可（起動時なのでパフォーマンス影響小）
```cpp
// エンジン初期化時（一度だけ実行される処理）
void Engine::Initialize() {
    if (!InitializeRenderer()) {
        throw std::runtime_error("Failed to initialize renderer");
    }
}
```

3. **従来型**: エラーコード（レガシー互換性が必要な場合）
```cpp
enum class InitResult { Success, Failed };
InitResult InitializeAudio() { /* ... */ }
```

#### 例外を避けるべき場所
- フレーム単位で呼ばれる処理（Update/Render）
- リアルタイム処理全般
- メモリ制約が厳しい環境

### プラットフォーム対応
```cpp
// プラットフォーム固有コードの分離
#ifdef ELK_PLATFORM_WINDOWS
    // Windows固有実装
#elif defined(ELK_PLATFORM_LINUX)
    // Linux固有実装
#elif defined(ELK_PLATFORM_MACOS)
    // macOS固有実装
#endif
```

## 📚 重要なファイル

### 設定ファイル
- `CMakeLists.txt`: ルートCMake設定
- `cmake/EngineUtils.cmake`: CMakeヘルパー関数
- `ThirdParty/CMakeLists.txt`: 外部ライブラリ管理

### APIヘッダ
- `ElkEngine/Public/ElkEngine/Core/Engine.h`: エンジンのメインAPI
- `ElkEngine/Public/ElkEngine/Core/Application.h`: アプリケーション基底クラス
- `ElkEngine/Public/ElkEngine/ECS/TypeRegistry.h`: 型登録システム
- `EditorBridge/Public/EditorBridge.h`: C#ブリッジAPI

### 実装ファイル
- `ElkEngine/Private/Core/Engine.cpp`: エンジン実装
- `ElkEngine/Private/ECS/TypeRegistry.cpp`: 型登録実装
- `EditorBridge/Private/EditorBridge.cpp`: ブリッジ実装

## 🎯 開発目標と優先順位

### Phase 1: 基盤（完了）
- ✅ プロジェクト構造
- ✅ CMake設定
- ✅ 基本Engine/Application
- ✅ ログシステム

### Phase 2: レンダリング（進行中）
- 🔄 RHI抽象化
- ⏳ DirectX 12実装
- ⏳ 基本的な描画
- ⏳ シェーダーシステム

### Phase 3: エディタ統合（計画中）
- ⏳ EditorBridge実装
- ⏳ C# WPFエディタ
- ⏳ ビューポート統合

## 🤖 AI開発者への具体的な指示

### コード生成時の注意点
1. **必ず既存のコーディング規約に従う**
2. **c++23の機能を積極的に活用する**
3. **ENGINE_API マクロを公開クラス/関数に付ける**
4. **エラーハンドリングにstd::expectedを使用する**
5. **ログ出力にはGAME_LOG_*マクロを使用する**

### ファイル作成時のテンプレート
```cpp
// ヘッダファイルのテンプレート
#pragma once

#include "ElkEngine/Core/EngineAPI.h"
#include <expected>

namespace elk::module_name {

// エラー型の定義
enum class InitError {
    InvalidConfiguration,
    ResourceNotFound,
    PermissionDenied
};

class ENGINE_API NewClass {
public:
    NewClass() = default;
    ~NewClass() = default;

    // 削除されたコピー/ムーブ（必要に応じて）
    NewClass(const NewClass&) = delete;
    NewClass& operator=(const NewClass&) = delete;

    // std::expected を使用したエラーハンドリング
    [[nodiscard]] std::expected<void, InitError> Initialize() noexcept;
    void Shutdown() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace elk::module_name
```

### CMake追加時のパターン
```cmake
# 新しいモジュール追加
elk_add_source_directory(MODULE_SOURCES "Private/NewModule")
list(APPEND ENGINE_ALL_SOURCES ${MODULE_SOURCES})

# ターゲット設定
elk_setup_library_properties(NewTarget)
elk_setup_compiler_warnings(NewTarget)
elk_print_target_info(NewTarget)
```

---

**このドキュメントを参考に、ElkGameEngineの設計思想と開発方針に沿った高品質なコードを生成してください。**