#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>
#include <mutex>

namespace elk::logger {
    /// <summary>
    /// タグ名と一意のID（TagId）との間の双方向マッピングを管理するクラスです。タグ名からIDの取得・登録、IDからタグ名の取得、登録済みタグ数の取得ができます。
    /// </summary>
    class TagRegistry {
    public:
        using TagId = uint32_t;

        TagRegistry() = default;
        
        /// <summary>
        /// 指定されたタグ文字列に対応するTagIdを取得し、未登録の場合は新たに登録してTagIdを返します。
        /// </summary>
        /// <param name="tag">取得または登録するタグ名を表す文字列ビュー。</param>
        /// <returns>タグに対応するTagId。タグが未登録の場合は新たに登録されたTagId。</returns>
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

        /// <summary>
        /// 指定されたTagIdに対応する文字列を返します。
        /// </summary>
        /// <param name="id">取得するタグID。</param>
        /// <returns>指定されたTagIdに対応する文字列への参照。</returns>
        const std::string& ToString(TagId id) const {
            return m_idToTag[id];
        }

        /// <summary>
        /// タグの総数を返します。
        /// </summary>
        /// <returns>現在のオブジェクトに関連付けられているタグの数。</returns>
        size_t TagCount() const {
            return m_idToTag.size();
        }

    private:
        /// <summary>
        /// std::string および std::string_view 型の文字列に対してハッシュ値を計算するファンクタです。異種検索（heterogeneous lookup）を有効にします。
        /// </summary>
        struct StringHash {
            using is_transparent = void; // heterogeneous lookup 有効化
            size_t operator()(std::string_view s) const noexcept {
                return std::hash<std::string_view>{}(s);
            }
            size_t operator()(const std::string& s) const noexcept {
                return std::hash<std::string>{}(s);
            }
        };

        /// <summary>
        /// 2つの文字列または文字列ビューが等しいかどうかを比較する関数オブジェクトです。
        /// </summary>
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

    private:
        std::unordered_map<std::string, TagId, StringHash, StringEqual> m_tagToId;
        std::vector<std::string> m_idToTag;
        mutable std::mutex m_mutex;
    };
} // namespace elk::logger
