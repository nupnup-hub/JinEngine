#include"JEditorAddressBar.h"
#include"../Gui/JGui.h" 
#include"../Gui/JGuiImageInfo.h"

namespace JinEngine::Editor
{
	JEditorAddressBar::JEditorAddressBar(const JUserPtr<JDirectory>& initDir)
		:initDir(initDir)
	{ 
		opendDirectory = initDir;
	}
	bool JEditorAddressBar::Update()
	{
		if (initDir == nullptr)
			return false;

		if (opendDirectory == nullptr)
			opendDirectory = initDir;

		std::vector<JUserPtr<JDirectory>> dirVec;
		JUserPtr<JDirectory> curr = opendDirectory;
		while (curr != nullptr)
		{
			dirVec.push_back(curr);
			curr = curr->GetParent();
			if (curr != nullptr && Core::HasSQValueEnum(curr->GetFlag(), OBJECT_FLAG_HIDDEN))
				curr = nullptr;
		}
		std::reverse(dirVec.begin(), dirVec.end());
		 
		const float height = JGui::GetAlphabetSize().y;
		const JVector2F size = JVector2F(JGui::GetWindowSize().x, height * 1.5f);

		bool isSelected = false;
		JGui::PushColorToSoft(J_GUI_COLOR::HEADER, JVector4F(0.1f));
		JGui::DrawRectFilledColor(JGui::GetCursorScreenPos(), size, JGui::GetColor(J_GUI_COLOR::HEADER), true, true);
		JGui::PopColor(1); 

		JGui::SetCursorPosX(JGui::GetCursorPosX() + JGui::GetFramePadding().x);
		JGui::SetCursorPosY(JGui::GetCursorPosY() + height * 0.25f);

		JGui::PushColor(J_GUI_COLOR::BUTTON, JGui::GetColor(J_GUI_COLOR::HEADER));
		JGui::PushColor(J_GUI_COLOR::BUTTON_ACTIVE, JGui::GetColor(J_GUI_COLOR::HEADER_ACTIVE));
		JGui::PushColor(J_GUI_COLOR::BUTTON_HOVERED, JGui::GetColor(J_GUI_COLOR::HEADER_HOVERED));

		const uint count = (uint)dirVec.size();
		for (uint i = 0; i < count; ++i)
		{
			JUserPtr<JDirectory> dir = dirVec[i];
			std::string name = JCUtil::WstrToU8Str(dir->GetName());
			JVector2F size = JGui::CalTextSize(name);
			if (JGui::Selectable(JGui::CreateGuiLabel(name, "EditorAddress"), false, J_GUI_SELECTABLE_DONT_CLOSE_POPUP, size) && opendDirectory != dirVec[i])
			{
				opendDirectory = dirVec[i];
				lastOpenList = nullptr;
				isSelected = true;
			}
			if (i != (count - 1))
			{
				JGui::SameLine();
				const JVector2F preCursorPos = JGui::GetCursorPos();
				auto childVec = dirVec[i]->GetChildDirctoryVec();
				if (JGui::BeginComboEx(JGui::CreateGuiLabel(dir->GetGuid(), "EditorAddressListBox"), "", J_GUI_COMBO_FLAG_HEIGHT_NO_PREVIEW, J_GUI_CARDINAL_DIR::RIGHT))
				{
					for (const auto& child : childVec)
					{
						if (JGui::Selectable(JGui::CreateGuiLabel(JCUtil::WstrToU8Str(child->GetName()), "EditorAddressList"), false, J_GUI_SELECTABLE_DONT_CLOSE_POPUP, size) && opendDirectory != child)
						{
							opendDirectory = child;
							isSelected = true;
						}
					}
					lastOpenList = dirVec[i];
					JGui::EndCombo();
				}
				if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT) && !JGui::IsMouseInRect(JGui::GetLastItemRectPos(), JGui::GetLastItemRectSize()))
					lastOpenList = nullptr;
				JGui::SetCursorPos(preCursorPos);
				JGui::SameLine();
			}
		} 
		JGui::PopColor(3);  
		return isSelected;
	}
	JDirectory* JEditorAddressBar::GetOpendDirectoryRaw()const noexcept
	{
		return opendDirectory.Get();
	}
	JUserPtr<JDirectory> JEditorAddressBar::GetOpendDirectory()const noexcept
	{
		return opendDirectory;
	} 
	void JEditorAddressBar::SetOpendDirectory(const JUserPtr<JDirectory>& dir)
	{
		opendDirectory = dir;
	}
}