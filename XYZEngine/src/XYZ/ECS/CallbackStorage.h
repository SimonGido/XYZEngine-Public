#pragma once
#include "XYZ/Utils/DataStructures/FreeList.h"

namespace XYZ {

	enum class CallbackType
	{
		None,
		ComponentCreate,
		ComponentRemove,
		EntityDestroy
	};

	struct Listener
	{
		std::function<void(uint32_t, CallbackType)> Callback;
		void* Instance;
	};

	class ICallbackStorage
	{
	public:
		virtual ~ICallbackStorage() = default;

		virtual void			  Clear() = 0;
		virtual void			  Move(uint8_t* buffer) = 0;
		virtual ICallbackStorage* Move() = 0;
		virtual ICallbackStorage* Copy(uint8_t* buffer) const = 0;
		virtual ICallbackStorage* Copy() const = 0;
		virtual void			  Execute(uint32_t entity, CallbackType type) = 0;
	};

	template <typename T>
	class CallbackStorage : public ICallbackStorage
	{
	public:
		CallbackStorage() = default;
		CallbackStorage(const CallbackStorage<T>& other)
			:
			m_Listeners(other.m_Listeners)
		{}

		CallbackStorage(CallbackStorage<T>&& other) noexcept
			:
			m_Listeners(std::move(other.m_Listeners))
		{}

		virtual void Clear() override
		{
			m_Listeners.clear();
		}
		virtual void Move(uint8_t* buffer) override
		{
			new (buffer)CallbackStorage<T>(std::move(*this));
		}
		virtual ICallbackStorage* Move() override
		{
			return new CallbackStorage<T>(std::move(*this));
		}
		virtual ICallbackStorage* Copy(uint8_t* buffer) const override
		{
			return new (buffer)CallbackStorage<T>(*this);
		}
		virtual ICallbackStorage* Copy() const override
		{
			return new CallbackStorage<T>(*this);
		}

		virtual void Execute(uint32_t entity, CallbackType type) override
		{
			for (auto& listener : m_Listeners)
			{
				listener.Callback(entity, type);
			}
		}

		void AddListener(const std::function<void(uint32_t, CallbackType)>& callback, void* instance)
		{
			XYZ_ASSERT(std::invoke([this, instance]() -> bool {
				for (auto it : m_Listeners)
				{
					if (it.Instance == instance)
						return false;
				}
				return true;
				}), "");
			m_Listeners.push_back({ callback, instance});
		}

		void RemoveListener(void *instance)
		{
			for (auto it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
			{
				if (it->Instance == instance)
				{
					m_Listeners.erase(it);
					return;
				}
			}
		}
		
	

	private:
		std::vector<Listener> m_Listeners;
	};

}