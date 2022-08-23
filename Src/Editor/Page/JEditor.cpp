#include"JEditor.h"
#include"JEditorAttribute.h"
#include"../../Utility/JCommonUtility.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"

namespace JinEngine
{
	namespace Editor
	{ 
		std::string JEditor::GetName()const noexcept
		{
			return attribute->name;
		}
		std::wstring JEditor::GetWName()const noexcept
		{
			return JCommonUtility::StringToWstring(attribute->name);
		}
		size_t JEditor::GetGuid()const noexcept
		{
			return attribute->guid;
		}
		float JEditor::GetInitPosXRate()const noexcept
		{
			return attribute->initPosXRate;
		}
		float JEditor::GetInitPsYRate()const noexcept
		{
			return attribute->initPosYRate;
		}
		float JEditor::GetInitWidthRate()const noexcept
		{
			return attribute->initWidthRate;
		}
		float JEditor::GetInitHeightRate()const noexcept
		{
			return attribute->initHeightRate;
		}
		bool* JEditor::GetOpenPtr()const noexcept
		{
			return &attribute->isOpen;
		}
		bool JEditor::IsOpen()const noexcept
		{
			return attribute->isOpen;
		}
		bool JEditor::IsFront()const noexcept
		{
			return attribute->isFront;
		}
		bool JEditor::IsFocus()const noexcept
		{
			return attribute->isFocus;
		}
		bool JEditor::IsActivated()const noexcept
		{
			return attribute->isActivated;
		}
		void JEditor::SetOpen()noexcept
		{
			if (!attribute->isOpen)
				DoSetOpen();			
		}
		void JEditor::OffOpen()noexcept
		{
			if (attribute->isOpen)
				DoOffOpen();
		}
		void JEditor::SetFront()noexcept
		{
			if (!attribute->isFront)
				DoSetFront();
		}
		void JEditor::OffFront()noexcept
		{
			if (attribute->isFront)
				DoOffFront();
		}
		void JEditor::SetFocus()noexcept
		{
			if (!attribute->isFocus)
				DoSetFocus();
		}
		void JEditor::OffFocus()noexcept
		{
			if (attribute->isFocus)
				DoOffFront();
		}
		void JEditor::Activate()noexcept
		{
			if (!attribute->isActivated)
				DoActivate();
		}
		void JEditor::DeActivate()noexcept
		{
			if (attribute->isActivated)
				DoDeActivate();
		}
		void JEditor::DoSetOpen()noexcept
		{
			attribute->isOpen = true;
		}
		void JEditor::DoOffOpen()noexcept
		{
			attribute->isOpen = false;
		}
		void JEditor::DoSetFront()noexcept
		{
			attribute->isFront = true;
		}
		void JEditor::DoOffFront()noexcept
		{
			attribute->isFront = false;
		}
		void JEditor::DoSetFocus()noexcept
		{
			attribute->isFocus = true;
		}
		void JEditor::DoOffFocus()noexcept
		{
			attribute->isFocus = false;
		}
		void JEditor::DoActivate()noexcept
		{
			attribute->isActivated = true;
		}
		void JEditor::DoDeActivate()noexcept
		{
			attribute->isActivated = false;
		}
		void JEditor::RegisterJFunc()
		{

		}
		JEditor::JEditor(std::unique_ptr<JEditorAttribute> attribute)
			:attribute(std::move(attribute))
		{}
		JEditor::~JEditor()
		{
			RemoveListener(*JImGuiImpl::EvInterface(), GetGuid());
		}
	}
}

/*
		bool JEditor::Activate(JEditorUtility* editorUtility)
		{
			if (!attribute->isActivated && attribute->isOpen && attribute->isFront)
			{
				attribute->isActivated = true;
				return true;
			}
			else
				return false;
		}
		bool JEditor::DeActivate(JEditorUtility* editorUtility)
		{
			if (attribute->isActivated && attribute->isOpen && attribute->isFront)
			{
				if (attribute->isFocus)
					OffFocus(editorUtility);
				attribute->isActivated = false;
				return true;
			}
			else
				return false;
		}
*/