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


#include"JEditorTreeStructure.h"
#include"../Gui/JGui.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Identity/JIdentifier.h"

namespace JinEngine::Editor
{
	void JEditorTreeStructure::Begin()
	{
		applyLoadedNode = loadedOpendNodeID.size() > 0;
	}
	void JEditorTreeStructure::End()
	{
		if (applyLoadedNode)
			loadedOpendNodeID.clear();
	}
	bool JEditorTreeStructure::DisplayTreeNode(const std::string& label, J_GUI_TREE_NODE_FLAG_ flags, const bool isFocus, const bool isActivated, const bool isSelected)
	{
		if (applyLoadedNode)
		{
			const GuiID id = JGui::CalCurrentWindowItemID(label);
			auto data = loadedOpendNodeID.find(id);
			if (data != loadedOpendNodeID.end())
				JGui::SetNextItemOpen(true);
		}
		JGui::PushStyle(J_GUI_STYLE::FRAME_ROUNDING, 0);
		JGui::PushStyle(J_GUI_STYLE::FRAME_BORDER_SIZE, 0);
		JGui::PushTreeNodeColorSet(isFocus, isActivated, isSelected);
		const bool isNodeOpen = JGui::TreeNodeEx(label, flags);
		JGui::PopTreeNodeColorSet(isActivated, isSelected);
		JGui::PopStyle(2);
		return isNodeOpen;
	}
	bool JEditorTreeStructure::CheckTreeNodeIsOpen(const std::string& label, J_GUI_TREE_NODE_FLAG_ flags, const bool isFocus, const bool isActivated, const bool isSelected)
	{
		if (applyLoadedNode)
		{
			const GuiID id = JGui::CalCurrentWindowItemID(label);
			auto data = loadedOpendNodeID.find(id);
			if (data != loadedOpendNodeID.end())
				JGui::SetNextItemOpen(true);
		}
		JGui::PushTreeNodeColorSet(isFocus, isActivated, isSelected);
		const bool isNodeOpen = JGui::IsTreeNodeOpend(label, flags);
		JGui::PopTreeNodeColorSet(isActivated, isSelected);
		return isNodeOpen;
	}
	void JEditorTreeStructure::TreePop()
	{
		JGui::TreePop();
	}
	void JEditorTreeStructure::LoadData(JFileIOTool& tool)
	{
		if (!tool.CanLoad())
			return;
		 
		uint opendNodeCount = 0;
		if (!tool.PushExistStack("TreeInfo"))
			return;

		JFileIOHelper::LoadAtomicData(tool, opendNodeCount, "OpendTreeNodeCount: ");
		tool.PushExistStack("OpendTreeNodeInfo"); 
		for (uint i = 0; i < opendNodeCount; ++i)
		{
			size_t id = 0;
			tool.PushExistStack();
			if (JFileIOHelper::LoadAtomicData(tool, id, "TreeNodeID") == Core::J_FILE_IO_RESULT::SUCCESS)
				loadedOpendNodeID.emplace(id);
			tool.PopStack();
		} 
		tool.PopStack(); 
		tool.PopStack();
	}
	void JEditorTreeStructure::StoreData(JFileIOTool& tool, const GuiID wndID)
	{
		if (!tool.CanStore())
			return;

		auto opendVec = JGui::GetWindowOpendTreeNodeID(wndID);
		tool.PushMember("TreeInfo");
		JFileIOHelper::StoreAtomicData(tool, opendVec.size(), "OpendTreeNodeCount: ");

		tool.PushArrayOwner("OpendTreeNodeInfo");
		for (const auto& data : opendVec)
		{
			tool.PushArrayMember();
			JFileIOHelper::StoreAtomicData(tool, data, "TreeNodeID");
			tool.PopStack();
		}	 
		tool.PopStack();
		tool.PopStack();
	}
	J_GUI_TREE_NODE_FLAG_ JEditorTreeStructure::GetBaseFlag()const noexcept
	{
		return J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW |
			J_GUI_TREE_NODE_FLAG_EXTEND_FULL_WIDTH |
			J_GUI_TREE_NODE_FLAG_FRAMED;
	}
}