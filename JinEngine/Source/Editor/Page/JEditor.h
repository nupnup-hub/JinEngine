#pragma once  
#include"../Event/JEditorEventType.h"
#include"../Event/JEditorEventStruct.h" 
#include"../../Core/JCoreEssential.h"  
#include"../../Core/Event/JEventListener.h"
#include"../../Core/File/JFileIOHelper.h"
#include<string>
#include<memory> 

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