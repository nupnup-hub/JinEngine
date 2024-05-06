/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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