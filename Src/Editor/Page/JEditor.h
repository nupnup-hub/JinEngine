#pragma once  
#include"../../Core/JDataType.h"
#include"../../Core/Reflection/JReflection.h"
#include<string>
#include<memory> 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorAttribute; 
		class JEditor
		{
			REGISTER_CLASS(JEditor)
		private:
			std::unique_ptr<JEditorAttribute> attribute;
		public:
			std::string GetName()const noexcept;
			std::wstring GetWName()const noexcept;
			size_t GetGuid()const noexcept;
			float GetInitPosXRate()const noexcept;
			float GetInitPsYRate()const noexcept;
			float GetInitWidthRate()const noexcept;
			float GetInitHeightRate()const noexcept;
			bool* GetOpenPtr()const noexcept;  
		public:
			bool IsOpen()const noexcept;
			bool IsFront()const noexcept;
			bool IsFocus()const noexcept;
			bool IsActivated()const noexcept;
		public:
			void SetOpen()noexcept;
			void OffOpen()noexcept;
			void SetFront()noexcept;
			void OffFront()noexcept;
			void SetFocus()noexcept;
			void OffFocus()noexcept;
			void Activate()noexcept;
			void DeActivate()noexcept;
		protected:
			virtual void DoSetOpen()noexcept;
			virtual void DoOffOpen()noexcept;
			virtual void DoSetFront()noexcept;
			virtual void DoOffFront()noexcept;
			virtual void DoSetFocus()noexcept;
			virtual void DoOffFocus()noexcept;
			virtual void DoActivate()noexcept;
			virtual void DoDeActivate()noexcept;
		protected:
			JEditor(std::unique_ptr<JEditorAttribute> attribute);
			~JEditor();
		};
	}
}