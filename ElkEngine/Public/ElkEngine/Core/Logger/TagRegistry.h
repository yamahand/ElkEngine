#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

namespace elk::logger {
    class TagRegistry {
    public:
        using TagId = uint32_t;

        static TagRegistry& Instance() {
            static TagRegistry instance;
            return instance;
        }

        // タグを登録 or 既存IDを取得
        TagId GetOrRegister(std::string_view tag) {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_tagToId.find(tag); // string_view で直接検索できる
            if (it != m_tagToId.end()) {
                return it->second;
            }

            TagId newId = static_cast<TagId>(m_idToTag.size());
            m_idToTag.push_back(std::string(tag));
            m_tagToId.emplace(m_idToTag.back(), newId);
            return newId;
        }

        // ID → string
        const std::string& ToString(TagId id) const {
            return m_idToTag[id];
        }

        // 登録されているタグ数を取得
        size_t TagCount() const {
            return m_idToTag.size();
        }

    private:
        TagRegistry() = default;

        // --- 透過検索用ハッシュ & イコライザ ---
        struct StringHash {
            using is_transparent = void; // heterogeneous lookup 有効化
            size_t operator()(std::string_view s) const noexcept {
                return std::hash<std::string_view>{}(s);
            }
            size_t operator()(const std::string& s) const noexcept {
                return std::hash<std::string>{}(s);
            }
        };

        struct StringEqual {
            using is_transparent = void;
            bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
                return lhs == rhs;
            }
            bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
                return lhs == rhs;
            }
            bool operator()(const std::string& lhs, std::string_view rhs) const noexcept {
                return lhs == rhs;
            }
        };

        std::unordered_map<std::string, TagId, StringHash, StringEqual> m_tagToId;
        std::vector<std::string> m_idToTag;
        mutable std::mutex m_mutex;
    };
} // namespace elk::logger
