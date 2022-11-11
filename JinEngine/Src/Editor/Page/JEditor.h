#pragma once  
#include"../Event/JEditorEventType.h"
#include"../Event/JEditorEventStruct.h" 
#include"../../Core/JDataType.h"  
#include"../../Core/Event/JEventListener.h"
#include<string>
#include<memory> 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute;  
		class JEditor : public Core::JEventListener<size_t, J_EDITOR_EVENT, JEditorEvStruct*>
		{  
		private: 
			std::string name;
			size_t guid;
			std::unique_ptr<JEditorAttribute> attribute;
		public:
			JEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute);
			virtual ~JEditor();
		public: 
			std::string GetName()const noexcept;
			std::string GetDockNodeName()const noexcept;
			size_t GetGuid()const noexcept;
			float GetInitPosXRate()const noexcept;
			float GetInitPosYRate()const noexcept;
			float GetInitWidthRate()const noexcept;
			float GetInitHeightRate()const noexcept;
			bool* GetOpenPtr()const noexcept;  

			void SetName(const std::string& newName)noexcept;
		public:
			bool IsOpen()const noexcept; 
			bool IsFocus()const noexcept;
			bool IsActivated()const noexcept;
			bool IsLastActivated()const noexcept;
		public:
			void SetOpen()noexcept;
			void SetClose()noexcept; 
			void SetFocus()noexcept;
			void SetUnFocus()noexcept;
			void Activate()noexcept;
			void DeActivate()noexcept;
		public:
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