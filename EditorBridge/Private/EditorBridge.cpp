#include "EditorBridge.h"

#include <string>
#include <vector>
#include <mutex>
#include <cstdlib> // malloc/free
#include <cstring> // strlen, memcpy
#include <cstdint>
#include <limits>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

// TypeRegistry を呼び出す（エンジン側のヘッダを参照）
#include "ElkEngine/ECS/TypeRegistry.h"

using elk::ecs::TypeRegistry;

// シンプルな内部オブジェクト（実装はプロジェクトに合わせて差し替え）
struct EngineOpaque {
    std::string lastError;
    std::mutex mtx;
    // ...実際のエンジンポインタや状態を格納...
};

struct ViewportOpaque {
    void* engine; // EngineOpaque*
#ifdef _WIN32
    HWND hwnd;
#endif
    int width;
    int height;
};

// ヘルパ: ブリッジ文字列を割り当てて返す（呼び出し側は FreeBridgeString で解放）
static const char* BridgeAllocString(const std::string& s) {
    size_t len = s.size();
    char* buf = (char*)std::malloc(len + 1);
    if (!buf) return nullptr;
    memcpy(buf, s.data(), len);
    buf[len] = '\0';
    return buf;
}

// FNV-1a 64bit ハッシュ（ランタイム版）
static uint64_t RuntimeFNV1a64(const char* s) {
    if (!s) return 0;
    uint64_t h = 14695981039346656037ULL;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(s);
    while (*p) {
        h ^= static_cast<uint64_t>(*p++);
        h *= 1099511628211ULL;
    }
    return h;
}

extern "C" {

BRIDGE_API void* CreateEditorEngine() {
    EngineOpaque* e = new EngineOpaque();
    return static_cast<void*>(e);
}

BRIDGE_API void DestroyEditorEngine(void* engine) {
    if (!engine) return;
    EngineOpaque* e = static_cast<EngineOpaque*>(engine);
    delete e;
}

BRIDGE_API bool InitializeEngine(void* engine, const char* configJson) {
    if (!engine) return false;
    EngineOpaque* e = static_cast<EngineOpaque*>(engine);
    std::lock_guard lk(e->mtx);
    // 簡易処理: 実装に合わせて初期化を行う
    // 失敗時は e->lastError に説明を入れて false を返す
    (void)configJson;
    return true;
}

BRIDGE_API void* CreateViewport(void* engine, intptr_t hwndPtr, int width, int height) {
    if (!engine) return nullptr;
    EngineOpaque* e = static_cast<EngineOpaque*>(engine);
    auto vp = new ViewportOpaque();
    vp->engine = engine;
#ifdef _WIN32
    vp->hwnd = reinterpret_cast<HWND>(hwndPtr);
#endif
    vp->width = width;
    vp->height = height;
    return static_cast<void*>(vp);
}

BRIDGE_API void ResizeViewport(void* viewport, int width, int height) {
    if (!viewport) return;
    ViewportOpaque* vp = static_cast<ViewportOpaque*>(viewport);
    vp->width = width;
    vp->height = height;
    // 実際のRHI/SwapChainリサイズ処理を呼ぶ
}

BRIDGE_API void RenderViewport(void* viewport) {
    if (!viewport) return;
    ViewportOpaque* vp = static_cast<ViewportOpaque*>(viewport);
    // レンダリング処理
    (void)vp;
}

BRIDGE_API void DestroyViewport(void* viewport) {
    if (!viewport) return;
    ViewportOpaque* vp = static_cast<ViewportOpaque*>(viewport);
    delete vp;
}

BRIDGE_API uint32_t CreateEntity(void* engine, const char* name) {
    if (!engine) return 0;
    // 仮実装: 常に 1 を返す。実エンジン実装に置換
    (void)name;
    return 1;
}

BRIDGE_API void DestroyEntity(void* engine, uint32_t entityId) {
    if (!engine) return;
    (void)entityId;
}

BRIDGE_API void AddComponent(void* engine, uint32_t entityId, const char* componentJson) {
    if (!engine) return;
    (void)entityId; (void)componentJson;
    // JSON 解析してコンポーネントを追加
}

BRIDGE_API bool ImportAsset(void* engine, const char* path, const char* type) {
    if (!engine) return false;
    (void)path; (void)type;
    return true;
}

BRIDGE_API const char* GetAssetList(void* engine, const char* filter) {
    if (!engine) return nullptr;
    EngineOpaque* e = static_cast<EngineOpaque*>(engine);
    std::lock_guard lk(e->mtx);
    // 仮: JSON 配列文字列を返す。実装ではアセットDBから組み立てる
    std::string result = R"(["asset1","asset2"])";
    (void)filter;
    return BridgeAllocString(result);
}

BRIDGE_API void FreeBridgeString(const char* str) {
    if (!str) return;
    std::free((void*)str);
}

BRIDGE_API const char* GetLastErrorString(void* engine) {
    if (!engine) return nullptr;
    EngineOpaque* e = static_cast<EngineOpaque*>(engine);
    std::lock_guard lk(e->mtx);
    if (e->lastError.empty()) return nullptr;
    return BridgeAllocString(e->lastError);
}

BRIDGE_API uint64_t EB_HashString(const char* str) {
    return RuntimeFNV1a64(str);
}

BRIDGE_API uint32_t EB_RegisterType(const char* stableName) {
    if (!stableName) return std::numeric_limits<uint32_t>::max();
    uint64_t h = RuntimeFNV1a64(stableName);
    return TypeRegistry::Instance().Register(h, stableName);
}

BRIDGE_API uint32_t EB_RegisterTypeWithHash(uint64_t stableHash, const char* stableName) {
    if (!stableName) return std::numeric_limits<uint32_t>::max();
    return TypeRegistry::Instance().Register(stableHash, stableName);
}

BRIDGE_API uint32_t EB_GetRuntimeTypeIdByHash(uint64_t stableHash) {
    return TypeRegistry::Instance().GetRuntimeId(stableHash);
}

} // extern "C"