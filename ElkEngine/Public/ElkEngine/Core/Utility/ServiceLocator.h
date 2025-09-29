#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <mutex>

namespace elk
{
	class ServiceLocator
	{
	public:
		
		/// <summary>
		/// サービスを登録します。
		/// </summary>
		/// <typeparam name="T">登録するサービスの型。</typeparam>
		/// <param name="service">登録するサービスの共有ポインタ。</param>
		template <typename T>
		static void Register(std::shared_ptr<T> service)
		{
			if(Has<T>())
			{
				// すでに登録されている場合は何もしない
				return;
			}

			std::lock_guard<std::mutex> lock(mutex_);
			services_[std::type_index(typeid(T))] = std::move(service);
		}

		/// <summary>
		/// 指定された型のサービスインスタンスを取得します。
		/// </summary>
		/// <typeparam name="T">取得するサービスの型。</typeparam>
		/// <returns>サービスが登録されていれば、その型の std::shared_ptr。登録されていなければ nullptr を返します。</returns>
		template <typename T>
		static std::shared_ptr<T> Get()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			auto it = services_.find(std::type_index(typeid(T)));
			if (it != services_.end())
			{
				return std::static_pointer_cast<T>(it->second);
			}
			return nullptr;
		}

		/// <summary>
		/// 指定された型のサービスを登録解除します。
		/// </summary>
		/// <typeparam name="T">登録解除するサービスの型。</typeparam>
		template <typename T>
		static void Unregister()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			services_.erase(std::type_index(typeid(T)));
		}

		/// <summary>
		/// サービスのコレクションをクリアし、すべてのサービスを削除します。スレッドセーフです。
		/// </summary>
		static void Clear()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			services_.clear();
		}

		/// <summary>
		/// 指定された型のサービスが登録されているかどうかを判定します。
		/// </summary>
		/// <typeparam name="T">判定するサービスの型。</typeparam>
		/// <returns>サービスが登録されていれば true、そうでなければ false を返します。</returns>
		template <typename T>
		static bool Has()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return services_.find(std::type_index(typeid(T))) != services_.end();
		}

	private:
		static inline std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
		static inline std::mutex mutex_;
	};
}