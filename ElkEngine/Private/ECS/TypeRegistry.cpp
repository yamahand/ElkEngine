#include "ElkEngine/ECS/TypeRegistry.h"

#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <string>

namespace elk::ecs {

struct TypeRegistry::Impl {
    mutable std::mutex mtx;
    std::unordered_map<uint64_t, uint32_t> stableToId;
    std::vector<std::string> idToName; // index 0 unused to keep 0 as invalid
    std::atomic<uint32_t> nextId{1};
    Impl() { idToName.emplace_back(); /* dummy for index 0 */ }
};

TypeRegistry& TypeRegistry::Instance() noexcept {
    static TypeRegistry inst;
    if (!inst.pimpl_) inst.pimpl_ = new Impl();
    return inst;
}

uint32_t TypeRegistry::Register(uint64_t stableHash, const char* stableName) noexcept {
    if (!pimpl_) return UINT32_MAX;
    std::lock_guard lk(pimpl_->mtx);
    auto it = pimpl_->stableToId.find(stableHash);
    if (it != pimpl_->stableToId.end()) return it->second;
    uint32_t id = pimpl_->nextId.fetch_add(1);
    pimpl_->stableToId.emplace(stableHash, id);
    pimpl_->idToName.emplace_back(stableName ? stableName : "");
    return id;
}

uint32_t TypeRegistry::GetRuntimeId(uint64_t stableHash) const noexcept {
    if (!pimpl_) return UINT32_MAX;
    std::lock_guard lk(pimpl_->mtx);
    auto it = pimpl_->stableToId.find(stableHash);
    return it != pimpl_->stableToId.end() ? it->second : UINT32_MAX;
}

const char* TypeRegistry::GetNameForRuntimeId(uint32_t runtimeId) const noexcept {
    if (!pimpl_) return nullptr;
    std::lock_guard lk(pimpl_->mtx);
    if (runtimeId == 0 || runtimeId >= pimpl_->idToName.size()) return nullptr;
    return pimpl_->idToName[runtimeId].c_str();
}

} // namespace elk::ecs