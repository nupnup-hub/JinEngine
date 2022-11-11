#include"JEditorEvent.h"    
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorEventManager : public Core::JEventManager<size_t, J_EDITOR_EVENT, JEditorEvStruct*>
		{
		public:
			std::deque<std::unique_ptr<JEditorEvStruct>> evQueue;
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
				SendEventNotification();
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
			ClearEvStructQueue();
		}
		JEditorEvStruct* JEditorEvent::RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct)noexcept
		{
			if (evStruct != nullptr)
			{
				JEditorEvStruct* ptr = evStruct.get();
				evM->evQueue.push_back(std::move(evStruct));
				return ptr;
			}
			else
				return nullptr;
		}
		void JEditorEvent::ClearEvStructQueue()noexcept
		{
			evM->evQueue.clear();
		}
		void JEditorEvent::Initialize()noexcept
		{
			if (evM == nullptr)
				evM = std::make_unique<JEditorEventManager>();
		}
		void JEditorEvent::Clear()noexcept
		{
			if (evM != nullptr)
			{
				ClearEvStructQueue();
				evM.reset();
			}
		}
	}
}