#pragma once

// ElkEngine/Public/ElkEngine/Core/EngineAPI.h
// ENGINE_API マクロの定義

// プラットフォーム検出
#if defined(_WIN32) || defined(_WIN64)
    #define ELK_PLATFORM_WINDOWS
#elif defined(__linux__)
    #define ELK_PLATFORM_LINUX
#elif defined(__APPLE__)
    #define ELK_PLATFORM_MACOS
#endif

// ENGINE_API マクロ定義
#if defined(ELK_PLATFORM_WINDOWS)
    // Windows での DLL エクスポート/インポート
    #if defined(BUILD_SHARED_LIBS) || defined(ENGINE_DLL)
        #if defined(ENGINE_EXPORTS)
            // DLL作成時（ENGINE_EXPORTS が定義されている場合）
            #define ENGINE_API __declspec(dllexport)
        #else
            // DLL使用時
            #define ENGINE_API __declspec(dllimport)
        #endif
    #else
        // 静的ライブラリの場合
        #define ENGINE_API
    #endif
#elif defined(ELK_PLATFORM_LINUX) || defined(ELK_PLATFORM_MACOS)
    // Linux/macOS での シンボル可視性
    #if defined(BUILD_SHARED_LIBS)
        #if defined(ENGINE_EXPORTS)
            #define ENGINE_API __attribute__((visibility("default")))
        #else
            #define ENGINE_API
        #endif
    #else
        // 静的ライブラリの場合
        #define ENGINE_API
    #endif
#else
    // その他のプラットフォーム
    #define ENGINE_API
#endif

// デバッグ用マクロ
#ifdef ELK_DEBUG
    #define ELK_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                fprintf(stderr, "Assertion failed: %s\nFile: %s\nLine: %d\nMessage: %s\n", \
                        #condition, __FILE__, __LINE__, message); \
                abort(); \
            } \
        } while(0)
#else
    #define ELK_ASSERT(condition, message) ((void)0)
#endif

// 便利マクロ
#define ELK_UNUSED(x) ((void)(x))

// 文字列化マクロ
#define ELK_STRINGIFY_IMPL(x) #x
#define ELK_STRINGIFY(x) ELK_STRINGIFY_IMPL(x)

#define ELK_ENGINE_VERSION_STRING \
    ELK_STRINGIFY(ELK_ENGINE_VERSION_MAJOR) "." \
    ELK_STRINGIFY(ELK_ENGINE_VERSION_MINOR) "." \
    ELK_STRINGIFY(ELK_ENGINE_VERSION_PATCH)

// elkネームスペースのエイリアス
namespace elk {}  // メインネームスペース