#!/usr/bin/env python3
"""
ElkGameEngine Project Setup Script

This script sets up the basic directory structure and creates template files
for a new ElkGameEngine project.
"""

import os
import sys
import argparse
from pathlib import Path
from typing import List, Dict

def create_directory_structure() -> Dict[str, List[str]]:
    """Define the directory structure to create."""
    structure = {
        "Assets": [
            "Models",
            "Textures", 
            "Audio",
            "Scenes",
            "Materials",
            "Scripts"
        ],
        "Documentation": [
            "API",
            "Design", 
            "UserGuide"
        ],
        "Editor/Source": [
            "GUI",
            "Connection",
            "AssetBrowser"
        ],
        "Editor/Resources": [],
        "ElkEngine/Public/ElkEngine": [
            "Core",
            "Renderer",
            "Audio", 
            "Input",
            "Math",
            "Platform"
        ],
        "ElkEngine/Private": [
            "Core",
            "Renderer",
            "Audio",
            "Input", 
            "Math",
            "Platforms/Windows",
            "Platforms/Linux",
            "Platforms/macOS"
        ],
        "Runtime/Source": [],
        "Samples": [
            "BasicGame",
            "PhysicsDemo"
        ],
        "Scripts": [],
        "ThirdParty": [],
        "Tools": [
            "AssetProcessor",
            "ShaderCompiler"
        ]
    }
    return structure

def create_template_files() -> Dict[str, str]:
    """Define template files to create."""
    templates = {}
    
    # Engine.h template
    templates["ElkEngine/Public/ElkEngine/Core/Engine.h"] = '''#pragma once

#include "EngineAPI.h"

namespace elk {

class Application;

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
    class EngineImpl* m_impl;
};

// Factory functions
ENGINE_API Engine* CreateEngine();
ENGINE_API void DestroyEngine(Engine* engine);

} // namespace elk
'''

    # Application.h template
    templates["ElkEngine/Public/ElkEngine/Core/Application.h"] = '''#pragma once

namespace elk {

class ENGINE_API Application {
public:
    Application() = default;
    virtual ~Application() = default;

    virtual bool Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    virtual void Shutdown() = 0;

    virtual const char* GetName() const = 0;
};

} // namespace elk
'''

    # main.cpp template
    templates["Runtime/Source/main.cpp"] = '''#include "ElkEngine/Core/Engine.h"
#include "GameApplication.h"

int main() {
    // Create engine
    auto* engine = elk::CreateEngine();
    if (!engine) {
        return -1;
    }

    // Initialize engine
    if (!engine->Initialize()) {
        elk::DestroyEngine(engine);
        return -1;
    }

    // Create game application
    auto game = std::make_unique<GameApplication>();

    // Run the engine
    engine->Run(game.get());

    // Cleanup
    elk::DestroyEngine(engine);
    return 0;
}
'''

    # GameApplication.h template
    templates["Runtime/Source/GameApplication.h"] = '''#pragma once

#include "ElkEngine/Core/Application.h"

class GameApplication : public elk::Application {
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
'''

    # GameApplication.cpp template
    templates["Runtime/Source/GameApplication.cpp"] = '''#include "GameApplication.h"
#include <iostream>

bool GameApplication::Initialize() {
    std::cout << "Game initialized!" << std::endl;
    return true;
}

void GameApplication::Update(float deltaTime) {
    // Update game logic
    (void)deltaTime; // Suppress unused parameter warning
}

void GameApplication::Render() {
    // Render game
}

void GameApplication::Shutdown() {
    std::cout << "Game shutdown!" << std::endl;
}
'''

    # Editor main.cpp template
    templates["Editor/Source/main.cpp"] = '''#include "EditorApplication.h"

int main() {
    EditorApplication editor;
    
    if (!editor.Initialize()) {
        return -1;
    }
    
    editor.Run();
    editor.Shutdown();
    
    return 0;
}
'''

    # EditorApplication.h template
    templates["Editor/Source/EditorApplication.h"] = '''#pragma once

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
'''

    # Tools CMakeLists.txt template
    templates["Tools/CMakeLists.txt"] = '''# Tools/CMakeLists.txt

message(STATUS "Configuring Tools...")

# Asset Processor
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/AssetProcessor")
    add_subdirectory(AssetProcessor)
endif()

# Shader Compiler
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ShaderCompiler")
    add_subdirectory(ShaderCompiler)
endif()

message(STATUS "Tools configuration completed")
'''

    return templates

def create_directories(base_path: Path, structure: Dict[str, List[str]]) -> None:
    """Create directory structure."""
    print("Creating directory structure...")
    
    for main_dir, subdirs in structure.items():
        main_path = base_path / main_dir
        main_path.mkdir(parents=True, exist_ok=True)
        print(f"  ✓ {main_dir}/")
        
        for subdir in subdirs:
            sub_path = main_path / subdir
            sub_path.mkdir(parents=True, exist_ok=True)
            print(f"    ✓ {main_dir}/{subdir}/")

def create_files(base_path: Path, templates: Dict[str, str]) -> None:
    """Create template files."""
    print("\nCreating template files...")
    
    for file_path, content in templates.items():
        full_path = base_path / file_path
        full_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Only create if file doesn't exist
        if not full_path.exists():
            with open(full_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ {file_path}")
        else:
            print(f"  ⚠ {file_path} (already exists, skipped)")

def create_gitkeep_files(base_path: Path, structure: Dict[str, List[str]]) -> None:
    """Create .gitkeep files in empty directories."""
    print("\nCreating .gitkeep files for empty directories...")
    
    empty_dirs = [
        "Assets/Models",
        "Assets/Textures", 
        "Assets/Audio",
        "Assets/Scenes",
        "Editor/Resources",
        "ThirdParty"
    ]
    
    for dir_path in empty_dirs:
        gitkeep_path = base_path / dir_path / ".gitkeep"
        gitkeep_path.parent.mkdir(parents=True, exist_ok=True)
        
        if not gitkeep_path.exists():
            gitkeep_path.touch()
            print(f"  ✓ {dir_path}/.gitkeep")

def setup_cmake_module_dirs(base_path: Path) -> None:
    """Create CMakeLists.txt files for modules that need them."""
    print("\nSetting up module CMakeLists.txt files...")
    
    # Samples CMakeLists.txt
    samples_cmake = base_path / "Samples" / "CMakeLists.txt"
    if not samples_cmake.exists():
        samples_cmake.parent.mkdir(parents=True, exist_ok=True)
        with open(samples_cmake, 'w') as f:
            f.write('''# Samples/CMakeLists.txt

message(STATUS "Configuring Samples...")

# Add sample projects here
# add_subdirectory(BasicGame)
# add_subdirectory(PhysicsDemo)

message(STATUS "Samples configuration completed")
''')
        print(f"  ✓ Samples/CMakeLists.txt")

def main():
    parser = argparse.ArgumentParser(description='Setup ElkGameEngine project structure')
    parser.add_argument('--path', '-p', type=str, default='.', 
                       help='Project root path (default: current directory)')
    parser.add_argument('--force', '-f', action='store_true',
                       help='Force overwrite existing files')
    
    args = parser.parse_args()
    
    project_path = Path(args.path).resolve()
    
    print(f"Setting up ElkGameEngine project at: {project_path}")
    print("=" * 60)
    
    # Create directory structure
    structure = create_directory_structure()
    create_directories(project_path, structure)
    
    # Create template files
    templates = create_template_files()
    create_files(project_path, templates)
    
    # Create .gitkeep files
    create_gitkeep_files(project_path, structure)
    
    # Setup additional CMake files
    setup_cmake_module_dirs(project_path)
    
    print("\n" + "=" * 60)
    print("✅ Project setup completed successfully!")
    print("\nNext steps:")
    print("1. Copy the CMakeLists.txt files from the artifacts")
    print("2. Run: Scripts/build.bat (Windows) or ./Scripts/build.sh (Linux/macOS)")
    print("3. Start developing your game engine!")
    print("\nFor more information, see README.md")

if __name__ == "__main__":
    main()