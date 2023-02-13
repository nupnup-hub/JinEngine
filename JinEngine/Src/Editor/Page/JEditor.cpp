#include"JEditor.h"
#include"JEditorAttribute.h"
#include"../Event/JEditorEvent.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditor::JEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute)
			:name(name),
			guid(JCUtil::CalculateGuid(name)),
			attribute(std::move(attribute))
		{}
		JEditor::~JEditor()
		{}
		std::string JEditor::GetName()const noexcept
		{
			return name;
		}
		std::string JEditor::GetDockNodeName()const noexcept
		{
			return GetName() + "DockNode";
		}
		size_t JEditor::GetGuid()const noexcept
		{
			return guid;
		}
		bool* JEditor::GetOpenPtr()const noexcept
		{
			return &attribute->isOpen;
		}
		void JEditor::SetName(const std::string& newName)noexcept
		{ 
			if(!newName.empty())
				name = newName;
		}
		bool JEditor::IsOpen()const noexcept
		{
			return attribute->isOpen;
		}
		bool JEditor::IsFocus()const noexcept
		{
			return attribute->isFocus;
		}
		bool JEditor::IsActivated()const noexcept
		{
			return attribute->isActivated;
		}
		bool JEditor::IsLastActivated()const noexcept
		{
			return attribute->isLastAct;
		}
		void JEditor::SetOpen()noexcept
		{
			if (!attribute->isOpen)
				DoSetOpen();			
		}
		void JEditor::SetClose()noexcept
		{
			if (attribute->isOpen)
				DoSetClose();
		}
		void JEditor::SetFocus()noexcept
		{
			if (!attribute->isFocus)
				DoSetFocus();
		}
		void JEditor::SetUnFocus()noexcept
		{
			if (attribute->isFocus)
				DoSetUnFocus();
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
		void JEditor::SetLastActivated(bool value)noexcept
		{
			attribute->isLastAct = value;
		}
		void JEditor::DoSetOpen()noexcept
		{
			attribute->isOpen = true;
		}
		void JEditor::DoSetClose()noexcept
		{
			attribute->isOpen = false;
		}
		void JEditor::DoSetFocus()noexcept
		{ 
			attribute->isFocus = true;
		}
		void JEditor::DoSetUnFocus()noexcept
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
		void JEditor::OnEvent(const size_t& iden, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{
			if (iden == guid)
				return;
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
					SetUnFocus(editorUtility);
				attribute->isActivated = false;
				return true;
			}
			else
				return false;
		}
*/