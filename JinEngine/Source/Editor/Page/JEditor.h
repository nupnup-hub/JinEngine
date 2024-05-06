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
#include"../Event/JEditorEventType.h"
#include"../Event/JEditorEventStruct.h" 
#include"../../Core/JCoreEssential.h"  
#include"../../Core/Event/JEventListener.h"
#include"../../Core/File/JFileIOHelper.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;  
		using EditorEventListener = Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEvStruct*>;
		class JEditor : public EditorEventListener
		{
		public:
			using EventF = Core::JSFunctorType<void, JEditor&, J_EDITOR_EVENT, JEditorEvStruct&>; 
		protected:
			using ClearTaskF = Core::JSFunctorType<void, std::vector<size_t>>;
		private: 
			std::string name;
			size_t guid;
			std::unique_ptr<JEditorAttribute> attribute;
		public:
			JEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute);
			virtual ~JEditor();
		public:
			bool IsOpen()const noexcept;
			//gui api에 update 상황에 따라 focus update하는 방식이 다르므로(실시간 반영, update end후 반영)
			//1frame 정도 늦은 focus정보를 갖고있다.
			bool IsFocus()const noexcept;
			bool IsActivated()const noexcept;
			bool IsLastActivated()const noexcept;
		public: 
			std::string GetName()const noexcept;
			std::string GetDockNodeName()const noexcept;
			size_t GetGuid()const noexcept; 
			bool* GetOpenPtr()const noexcept;  
			EventF::Functor* GetEvFunctor()noexcept;
		protected:
			ClearTaskF::Functor* GetClearTaskFunctor()noexcept;
		public:
			void SetName(const std::string& newName)noexcept;
		protected:
			//Support undo redo
			void RequestOpenPage(const JEditorOpenPageEvStruct& evStruct, const bool doAct);
			void RequestClosePage(const JEditorClosePageEvStruct& evStruct, const bool isAct);
		public:
			static size_t CalculateGuid(const std::string& str);
		public:
			void SetOpen()noexcept;
			void SetClose()noexcept;
			void SetFocus()noexcept;
			void SetUnFocus()noexcept;
			void Activate()noexcept;
			void DeActivate()noexcept;
			void SetLastActivated(bool value)noexcept;
		protected:
			virtual void DoSetOpen()noexcept;
			virtual void DoSetClose()noexcept;
			virtual void DoSetFocus()noexcept;
			virtual void DoSetUnFocus()noexcept;
			virtual void DoActivate()noexcept;
			virtual void DoDeActivate()noexcept;
		protected:
			void OnEvent(const size_t& iden, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)override;
		};
	}
}