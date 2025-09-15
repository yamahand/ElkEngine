# ElkGameEngine

CMake を採用したモダンな C++ 製のクロスプラットフォーム対応ゲームエンジンです。

## 🎯 特長

- **クロスプラットフォーム**: Windows、Linux、macOS をサポート
- **モダン C++17**: メンテナンスしやすいコードベース
- **モジュラー構造**: Core、Renderer、Audio、Input 等の分離されたモジュール
- **複数のグラフィックス API**: DirectX 11、OpenGL（将来的に Vulkan を予定）
- **統合エディタ**: アセット管理等を行う組み込みエディタ
- **CMake ビルドシステム**: 依存関係管理とビルドが容易
- **サードパーティ統合**: GLFW、GLM、STB、Dear ImGui など

## 📁 プロジェクト構成

```
ElkGameEngine/
├── Assets/                 # ゲーム用アセット
├── Documentation/          # ドキュメント
├── Editor/                 # エディタアプリケーション
├── ElkEngine/              # エンジンコア
│   ├── Public/             # 公開 API ヘッダ
│   └── Private/            # 実装詳細
├── Runtime/                # ランタイムアプリケーション
├── Samples/                # サンプルプロジェクト
├── Scripts/                # ビルドスクリプトなど
├── ThirdParty/             # 外部ライブラリ
└── Tools/                  # 開発ツール
```

## 🚀 クイックスタート

### 前提条件

- **Windows**: Visual Studio 2019 以降
- **Linux**: GCC 9+ または Clang 10+
- **macOS**: Xcode 12+ または Command Line Tools
- **CMake**: 3.20 以上
- **Git**: 依存関係管理に使用

### ビルド

#### Windows
```batch
# リポジトリをクローン
git clone https://github.com/yourusername/ElkGameEngine.git
cd ElkGameEngine

# デフォルト（Debug）でビルド
Scripts\build.bat

# Release ビルド（クリーンしてビルド）
Scripts\build.bat clean release

# Visual Studio で開く
Scripts\build.bat open
```

#### Linux / macOS
```bash
# リポジトリをクローン
git clone https://github.com/yourusername/ElkGameEngine.git
cd ElkGameEngine

# ビルドスクリプトに実行権限を付与
chmod +x Scripts/build.sh

# デフォルト（Debug）でビルド
./Scripts/build.sh

# Release ビルド（クリーンしてビルド）
./Scripts/build.sh clean release
```

### 実行

ビルド後、実行ファイルは以下に生成されます:

- **Windows**: `build/Debug/bin/` または `build/Release/bin/`
- **Linux/macOS**: `build/bin/`

```bash
# ランタイムを実行
cd build/Debug/bin  # または build/Release/bin
./Runtime

# エディタを実行
./Editor
```

## 🛠️ ビルドオプション

### CMake オプション

| オプション | デフォルト | 説明 |
|-----------:|:----------:|:-----|
| `BUILD_SHARED_LIBS` | `OFF` | 共有ライブラリ（DLL）としてビルドするか |
| `BUILD_EDITOR` | `ON` | エディタをビルドするか |
| `BUILD_TOOLS` | `ON` | 開発ツールをビルドするか |
| `BUILD_SAMPLES` | `OFF` | サンプルをビルドするか |
| `ELK_ENABLE_LOGGING` | `ON` | ロギングを有効にするか |
| `ELK_ENABLE_PROFILER` | `ON` | プロファイラを有効にするか（主に Debug 用） |

### カスタムビルド例

```bash
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_SAMPLES=ON

cmake --build . --config Release
```

## 🏗️ アーキテクチャ

### エンジンモジュール

- **Core**: アプリケーションフレームワーク、オブジェクトシステム、メモリ管理
- **Renderer**: 描画抽象化レイヤ（DirectX、OpenGL）
- **Audio**: オーディオ統合（予定）
- **Input**: 入力デバイス管理
- **Math**: 数学ユーティリティ
- **Platform**: プラットフォーム固有の実装

### API 使用例

```cpp
#include "ElkEngine/ElkEngine.h"

class MyGame : public elk::Application {
public:
    bool Initialize() override {
        // ゲーム初期化
        return true;
    }

    void Update(float deltaTime) override {
        // 更新処理
    }

    void Render() override {
        // 描画処理
    }
};

int main() {
    auto engine = elk::CreateEngine();
    auto game = std::make_unique<MyGame>();

    engine->Run(game.get());

    elk::DestroyEngine(engine);
    return 0;
}
```

## 🔧 開発

### 新しいモジュールの追加

1. `ElkEngine/Public/ElkEngine/NewModule/` と `ElkEngine/Private/NewModule/` を作成
2. `ElkEngine/CMakeLists.txt` に追加
3. Visual Studio 用のソースグループを設定

### サードパーティ

CMake の FetchContent で自動取得するか、`ThirdParty/` に手動配置します。

統合済みライブラリ:
- **GLFW**: ウィンドウ管理
- **GLM**: 数学
- **STB**: 画像等のユーティリティ
- **Dear ImGui**: デバッグ GUI

### プラットフォーム追加

1. `ElkEngine/Private/Platforms/YourPlatform/` に実装を追加
2. ルート `CMakeLists.txt` の検出ロジックを更新
3. `cmake/EngineUtils.cmake` に必要設定を追加

## 📚 ドキュメント

- [Build Guide](Documentation/BuildGuide.md)
- [Architecture Overview](Documentation/Architecture.md)
- [API Reference](Documentation/API.md)
- [Contributing Guidelines](Documentation/Contributing.md)

## 🤝 貢献

1. リポジトリを fork
2. ブランチを切る (`git checkout -b feature/amazing-feature`)
3. 変更をコミット (`git commit -m 'Add amazing feature'`)
4. ブランチを push (`git push origin feature/amazing-feature`)
5. プルリクエストを作成

貢献前に `Documentation/Contributing.md` をご確認ください。

## 📄 ライセンス

MIT ライセンス（詳細は `LICENSE` を参照）

## 🙏 謝辞

- [GLFW](https://www.glfw.org/)
- [GLM](https://glm.g-truc.net/)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [STB](https://github.com/nothings/stb)

## 📞 サポート

- **Issues**: https://github.com/yourusername/ElkGameEngine/issues
- **Discussions**: https://github.com/yourusername/ElkGameEngine/discussions
- **Wiki**: https://github.com/yourusername/ElkGameEngine/wiki

---

Made with ❤️ by the ElkGameEngine team
