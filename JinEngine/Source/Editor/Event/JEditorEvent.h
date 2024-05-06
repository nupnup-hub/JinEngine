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
			template<typename T>
			static auto RegisterEvStruct(std::unique_ptr<T> evStruct, _Out_ size_t& key, bool controlDestroyTiming)
				->typename Core::TypeCondition<T*, std::is_base_of_v<JEditorEvStruct, T>>::Type
			{ 
				return static_cast<T*>(_RegisterEvStruct(std::move(evStruct), key, controlDestroyTiming));
			}
		private:
			static JEditorEvStruct* _RegisterEvStruct(std::unique_ptr<JEditorEvStruct> evStruct, _Out_ size_t& key, bool controlDestroyTiming)noexcept;
		public:
			static void Initialize()noexcept;
			static void Clear()noexcept;
		public:
			static void SetCanDestroyBit(const size_t guid, const bool value)noexcept;
		};
	}
}