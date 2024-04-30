#include"JEditorEvent.h"    
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorEventManager : public Core::JEventManager<size_t, J_EDITOR_EVENT, JEditorEvStruct*>
		{
		public:
			struct EvStructInfo
			{
			public:
				std::unique_ptr<JEditorEvStruct> evStruct;
				bool canDestroyThisFrame = true;
			public:
				EvStructInfo(std::unique_ptr<JEditorEvStruct> evStruct, bool canDestroyThisFrame)
					:evStruct(std::move(evStruct)), canDestroyThisFrame(canDestroyThisFrame)
				{}
			};
		public:
			std::unordered_map<size_t, EvStructInfo> evDataMap; 
		public:
			JEditorEventManager()
				:JEventManager([](const size_t& a, const size_t& b) {return a == b; })
			{}
			~JEditorEventManager()
			{
				ClearEvent();
			}
		public:
			JEventInterface* EvInterface()final
			{
				return this;
			}
		public:
			void OnEvnet()
			{
				std::vector<size_t> eraseEv;
				for (const auto& data : evDataMap)
				{
					if (data.second.canDestroyThisFrame)
						eraseEv.push_back(data.first);
				} 
				SendEventNotification(); 
				for (const auto& data : eraseEv)
					evDataMap.erase(data);
			}
		};

		namespace
		{
			static std::unique_ptr<JEditorEventManager> evM;
		}

		JEditorEvent::JEvInterface* JEditorEvent::EvInterface()noexcept
		{
			return evM.get();
		}
		void JEditorEvent::ExecuteEvent()noexcept
		{
			evM->OnEvnet(); 
		}
		JEditorEvStruct* JEditorEvent::RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct)noexcept
		{
			size_t temp = 0;
			return RegisterEvStruct(std::move(evStruct), temp, false);
		}
		JEditorEvStruct* JEditorEvent::_RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct, _Out_ size_t& key, bool controlDestroyTiming)noexcept
		{
			key = 0;
			if (evStruct != nullptr)
			{
				key = Core::MakeGuid();
				JEditorEvStruct* ptr = evStruct.get();
				evM->evDataMap.emplace(key, JEditorEventManager::EvStructInfo(std::move(evStruct), !controlDestroyTiming));
				return ptr;
			}
			else
				return nullptr;
		}
		void JEditorEvent::Initialize()noexcept
		{
			if (evM == nullptr)
				evM = std::make_unique<JEditorEventManager>();
		}
		void JEditorEvent::Clear()noexcept
		{
			if (evM != nullptr)
				evM.reset();
		}
		void JEditorEvent::SetCanDestroyBit(const size_t guid, const bool value)noexcept
		{
			auto data = evM->evDataMap.find(guid);
			if (data != evM->evDataMap.end())
				data->second.canDestroyThisFrame = value;
		}
	}
}