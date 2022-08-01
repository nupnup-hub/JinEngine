#include"EditorPopup.h"
#include"EditorPopupNode.h"
#include"../../../Core/JDataType.h"  
#include"../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	EditorPopup::EditorPopup(const std::string& name, std::unique_ptr<EditorPopupNode> popupRoot)
		:name(name), popupRoot(popupRoot.get()), isOpen(false)
	{
		allPopupNode.push_back(std::move(popupRoot));
	}
	EditorPopup::~EditorPopup() {}
	void EditorPopup::AddPopupNode(std::unique_ptr<EditorPopupNode> child)noexcept
	{
		if (child != nullptr)
			allPopupNode.push_back(std::move(child));
	}
	void EditorPopup::ExecutePopup(_In_ EditorString* editorString, _Out_ EDITOR_POPUP_NODE_RES& res, _Out_ size_t& clickMenuGuid)noexcept
	{
		res = EDITOR_POPUP_NODE_RES::NON_CLICK;
		clickMenuGuid = 0;
		if (isOpen)
		{
			ImGui::OpenPopup(name.c_str());
			if (ImGui::BeginPopup(name.c_str()))
			{			 
				popupRoot->PopupOnScreen(editorString, res, clickMenuGuid);
				ImGui::Separator();
				ImGui::EndPopup();
			}
		}
	}
	bool EditorPopup::IsOpen()const noexcept
	{
		return isOpen;
	}
	bool EditorPopup::IsMouseInPopup()noexcept
	{
		if (!IsOpen())
			return false;
		else
		{  
			const uint allNodeCount = (uint)allPopupNode.size();
			for (uint i = 0; i < allNodeCount; ++i)
			{
				const EDITOR_POPUP_NODE_TYPE type = allPopupNode[i]->GetNodeType();
				if (type == EDITOR_POPUP_NODE_TYPE::ROOT)
				{
					bool res = allPopupNode[i]->IsMouseInPopup();
					if (res)
						return true;
				}
				else if (type == EDITOR_POPUP_NODE_TYPE::INTERNAL && allPopupNode[i]->IsOpen())
				{
					bool res = allPopupNode[i]->IsMouseInPopup();
					if (res)
						return true;
				}
			}
			return false;
		}
	}
	void EditorPopup::SetOpen(bool value)noexcept
	{
		isOpen = value;
	}

}