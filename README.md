# ElkGameEngine

[日本語版 README](README.ja.md)

A modern C++ game engine built with CMake, designed for cross-platform game development.

## 🎯 Features

- **Cross-Platform**: Windows, Linux, macOS support
- **Modern C++17**: Clean, maintainable codebase
- **Modular Architecture**: Core, Renderer, Audio, Input systems
- **Multiple Graphics APIs**: DirectX 11, OpenGL support (Vulkan planned)
- **Integrated Editor**: Built-in game editor for asset management
- **CMake Build System**: Easy dependency management and building
- **Third-Party Integration**: GLFW, GLM, STB, Dear ImGui

## 📁 Project Structure

```
ElkGameEngine/
├── Assets/                 # Game development assets
├── Documentation/          # Project documentation
├── Editor/                 # Game editor application
├── ElkEngine/             # Engine core library
│   ├── Public/            # Public API headers
│   └── Private/           # Internal implementation
├── Runtime/               # Game runtime application
├── Samples/               # Example projects
├── Scripts/               # Build scripts
├── ThirdParty/           # External libraries
└── Tools/                # Development tools
```

## 🚀 Quick Start

### Prerequisites

- **Windows**: Visual Studio 2019 or later
- **Linux**: GCC 9+ or Clang 10+
- **macOS**: Xcode 12+ or Command Line Tools
- **CMake**: 3.20 or later
- **Git**: For dependency management

### Building

#### Windows
```batch
# Clone the repository
git clone https://github.com/yourusername/ElkGameEngine.git
cd ElkGameEngine

# Build (Debug configuration)
Scripts\build.bat

# Build Release configuration
Scripts\build.bat clean release

# Open in Visual Studio
Scripts\build.bat open
```

#### Linux/macOS
```bash
# Clone the repository
git clone https://github.com/yourusername/ElkGameEngine.git
cd ElkGameEngine

# Make build script executable
chmod +x Scripts/build.sh

# Build (Debug configuration)
./Scripts/build.sh

# Build Release configuration
./Scripts/build.sh clean release
```

### Running

After building, executables will be available in:
- **Windows**: `build/Debug/bin/` or `build/Release/bin/`
- **Linux/macOS**: `build/bin/`

```bash
# Run the runtime
cd build/Debug/bin  # or build/Release/bin
./Runtime

# Run the editor
./Editor
```

## 🛠️ Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `OFF` | Build as shared libraries (DLL) |
| `BUILD_EDITOR` | `ON` | Build the game editor |
| `BUILD_TOOLS` | `ON` | Build development tools |
| `BUILD_SAMPLES` | `OFF` | Build sample projects |
| `ELK_ENABLE_LOGGING` | `ON` | Enable logging system |
| `ELK_ENABLE_PROFILER` | `ON` | Enable profiler (Debug only) |

### Custom Build

```bash
# Custom CMake configuration
mkdir build && cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_SAMPLES=ON

cmake --build . --config Release
```

## 🏗️ Architecture

### Engine Modules

- **Core**: Application framework, object system, memory management
- **Renderer**: Graphics abstraction layer (DirectX, OpenGL)
- **Audio**: Audio engine integration (planned)
- **Input**: Input device management
- **Math**: Mathematical utilities and types
- **Platform**: Platform-specific implementations

### API Example

```cpp
#include "ElkEngine/ElkEngine.h"

class MyGame : public ElkEngine::Application {
public:
    bool Initialize() override {
        // Initialize your game
        return true;
    }
    
    void Update(float deltaTime) override {
        // Update game logic
    }
    
    void Render() override {
        // Render your game
    }
};

int main() {
    auto engine = ElkEngine::CreateEngine();
    auto game = std::make_unique<MyGame>();
    
    engine->Run(game.get());
    
    ElkEngine::DestroyEngine(engine);
    return 0;
}
```

## 🔧 Development

### Adding New Modules

1. Create directories in `ElkEngine/Public/ElkEngine/NewModule/` and `ElkEngine/Private/NewModule/`
2. Update `ElkEngine/CMakeLists.txt` to include the new module
3. Add appropriate source groups for Visual Studio

### Third-Party Libraries

Libraries are managed via CMake FetchContent for automatic downloading, or can be placed manually in the `ThirdParty/` directory.

Currently integrated:
- **GLFW**: Window management
- **GLM**: Mathematics library
- **STB**: Image loading
- **Dear ImGui**: Debug UI

### Platform Support

New platforms can be added by:
1. Creating implementation in `ElkEngine/Private/Platforms/YourPlatform/`
2. Updating CMake detection in root `CMakeLists.txt`
3. Adding platform-specific libraries in `cmake/EngineUtils.cmake`

## 📚 Documentation

- [Build Guide](Documentation/BuildGuide.md)
- [Architecture Overview](Documentation/Architecture.md)
- [API Reference](Documentation/API.md)
- [Contributing Guidelines](Documentation/Contributing.md)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

Please read our [Contributing Guidelines](Documentation/Contributing.md) for details on our code of conduct and development process.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- [GLFW](https://www.glfw.org/) - OpenGL context and window management
- [GLM](https://glm.g-truc.net/) - Mathematics library
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [STB](https://github.com/nothings/stb) - Single-file public domain libraries

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/ElkGameEngine/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/ElkGameEngine/discussions)
- **Wiki**: [Project Wiki](https://github.com/yourusername/ElkGameEngine/wiki)

---

Made with ❤️ by the ElkGameEngine team