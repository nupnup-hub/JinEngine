#pragma once
#include"JEditorEventType.h"   
#include"JEditorEventStruct.h" 
#include"../../Core/Event/JEventListener.h"   
#include"../../Core/Event/JEventManager.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorEvent
		{
		private:
			using JEvInterface = Core::JEventInterface<size_t, J_EDITOR_EVENT, JEditorEvStruct*>;
		public:
			static JEvInterface* EvInterface()noexcept;
		public:
			static void ExecuteEvent()noexcept;
		public:
			static JEditorEvStruct* RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct)noexcept;
			static JEditorEvStruct* RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct, _Out_ size_t& key, bool canRemvoe = false)noexcept;
			static void ClearInvalidEvStructData()noexcept;
		public:
			static void Initialize()noexcept;
			static void Clear()noexcept;
		public:
			static void SetCanDestroyBit(const size_t guid, const bool value)noexcept;
		};
	}
}