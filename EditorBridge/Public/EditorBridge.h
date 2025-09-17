#pragma once

#include <stdint.h>

#ifndef BRIDGE_API
#ifdef _WIN32
#define BRIDGE_API __declspec(dllexport)
#else
#define BRIDGE_API
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 明確な所有権ルール:
// - エンジン/ビューポート等は不透明ハンドル(void*)で表現。
// - 文字列は呼び出し側が FreeBridgeString() を必ず呼んで解放すること。
// - HWND は intptr_t 経由で渡す（C# 側では IntPtr）。
//
// 呼び出しは可能な限りメインスレッドで行うこと（ビューポート関連はUIスレッド依存）。

// エンジン管理
BRIDGE_API void* CreateEditorEngine();
BRIDGE_API void DestroyEditorEngine(void* engine);
BRIDGE_API bool InitializeEngine(void* engine, const char* configJson);

// ビューポート管理
// hwnd は intptr_t（C# では IntPtr）で渡す
BRIDGE_API void* CreateViewport(void* engine, intptr_t hwnd, int width, int height);
BRIDGE_API void ResizeViewport(void* viewport, int width, int height);
BRIDGE_API void RenderViewport(void* viewport);
BRIDGE_API void DestroyViewport(void* viewport);

// ECS操作
BRIDGE_API uint32_t CreateEntity(void* engine, const char* name);
BRIDGE_API void DestroyEntity(void* engine, uint32_t entityId);
BRIDGE_API void AddComponent(void* engine, uint32_t entityId, const char* componentJson);

// アセット管理
BRIDGE_API bool ImportAsset(void* engine, const char* path, const char* type);
// GetAssetList はヒープ割当てされた null 終端 UTF-8 文字列を返す。
// 呼び出し側は FreeBridgeString() を使って必ず解放すること。
BRIDGE_API const char* GetAssetList(void* engine, const char* filter);

// メモリ解放: ブリッジ側で割当てた文字列を解放する
BRIDGE_API void FreeBridgeString(const char* str);

// エラー情報取得（直近のエラー説明を返す。こちらも FreeBridgeString で解放）
BRIDGE_API const char* GetLastErrorString(void* engine);

// 型登録 API - エディタから型名を登録して実行時IDを取得する
// - EB_RegisterType: stableName からハッシュを計算して登録。
// - EB_RegisterTypeWithHash: 既にハッシュを持っている場合に直接登録。
// - EB_GetRuntimeTypeIdByHash: stableHash から runtime id を取得（未登録なら UINT32_MAX）。
// - EB_HashString: FNV-1a 64bit ハッシュを返す（エディタ側で stableHash を事前計算する場合に利用可能）。
BRIDGE_API uint32_t EB_RegisterType(const char* stableName);
BRIDGE_API uint32_t EB_RegisterTypeWithHash(uint64_t stableHash, const char* stableName);
BRIDGE_API uint32_t EB_GetRuntimeTypeIdByHash(uint64_t stableHash);
BRIDGE_API uint64_t EB_HashString(const char* str);

#ifdef __cplusplus
} // extern "C"
#endif