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
		JGui::PushTreeNodeColorSet(isFocus, isActivated, isSelected);
		const bool isNodeOpen = JGui::TreeNodeEx(label, flags);
		JGui::PopTreeNodeColorSet(isActivated, isSelected);
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
}