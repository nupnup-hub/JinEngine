#pragma once 
#include"../../../Core/JDataType.h"
#include<string>
#include<memory>
#include<vector>

namespace JinEngine
{
	class EditorAttribute;
	class EditorUtility;
	class Editor
	{
	private:
		std::unique_ptr<EditorAttribute> attribute;
	public:
		Editor(std::unique_ptr<EditorAttribute> attribute);
		virtual ~Editor();
		Editor(const Editor& rhs) = delete;
		Editor& operator=(const Editor& rhs) = delete;

		virtual bool Activate(EditorUtility* editorUtility);
		virtual bool DeActivate(EditorUtility* editorUtility);
		virtual bool OnFocus(EditorUtility* editorUtility);
		virtual bool OffFocus(EditorUtility* editorUtility);
		bool SetFront(EditorUtility* editorUtility);
		bool OffFront(EditorUtility* editorUtility);
		bool Open(EditorUtility* editorUtility);
		bool Close(EditorUtility* editorUtility);

		float GetInitPosXRate()const noexcept;
		float GetInitPsYRate()const noexcept;
		float GetInitWidthRate()const noexcept;
		float GetInitHeightRate()const noexcept;
		bool* GetOpenPtr()const noexcept;
		bool* GetActivationPtr()const noexcept;

		bool IsOpen()const noexcept;
		bool IsFront()const noexcept;
		bool IsActivated()const noexcept;
		bool IsFocus()const noexcept;
		void SetOpen()noexcept;
		void SetFront()noexcept;
		void SetActivattion()noexcept;
		void OffOpen()noexcept;
		void OffFront()noexcept;
		void OffActivattion()noexcept;

		std::string GetName()const noexcept;
		std::wstring GetWName()const noexcept;
		size_t GetGuid()const noexcept;	 
	};
}