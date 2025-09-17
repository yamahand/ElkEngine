#pragma once

#include <cstdint>
#include <string>

namespace elk::ecs {

// constexpr FNV-1a 64bit
constexpr uint64_t ConstexprHashImpl(const char* s, uint64_t hash = 14695981039346656037ULL) {
    return (*s) ? ConstexprHashImpl(s + 1, (hash ^ static_cast<uint64_t>(static_cast<unsigned char>(*s))) * 1099511628211ULL) : hash;
}
constexpr uint64_t ConstexprHash(const char* s) { return ConstexprHashImpl(s); }

// TypeRegistry: stableHash(uint64) -> runtimeId(uint32)
class TypeRegistry {
public:
    static TypeRegistry& Instance() noexcept;

    // 登録: stableHash と stableName を渡す。既に登録済なら既存の runtime id を返す。
    uint32_t Register(uint64_t stableHash, const char* stableName) noexcept;

    // stableHash から runtime id を得る。未登録なら UINT32_MAX を返す。
    uint32_t GetRuntimeId(uint64_t stableHash) const noexcept;

    // 名前取得（主にデバッグ）
    const char* GetNameForRuntimeId(uint32_t runtimeId) const noexcept;

private:
    TypeRegistry() = default;
    ~TypeRegistry() = default;
    TypeRegistry(const TypeRegistry&) = delete;
    TypeRegistry& operator=(const TypeRegistry&) = delete;

    struct Impl;
    Impl* pimpl_ = nullptr;
};

} // namespace elk::ecs

// 利便性マクロ: 式として使える。例: uint32_t id = ELK_REGISTER_TYPE("elk::ecs::Transform");
#define ELK_REGISTER_TYPE(STABLE_NAME_LITERAL) \
    ( ::elk::ecs::TypeRegistry::Instance().Register(::elk::ecs::ConstexprHash(STABLE_NAME_LITERAL), STABLE_NAME_LITERAL) )

// 既に登録済みIDを取得する（未登録の場合は UINT32_MAX）
#define ELK_GET_RUNTIME_ID_BY_HASH(HASH) (::elk::ecs::TypeRegistry::Instance().GetRuntimeId(HASH))
