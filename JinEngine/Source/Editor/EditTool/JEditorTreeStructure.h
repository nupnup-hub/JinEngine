#pragma once
#include"../Gui/JGuiType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Reflection/JReflection.h" 
#include"../../Core/Interface/JTreeInterface.h" 
#include<set>

namespace JinEngine
{
	class JFileIOTool;
	namespace Core
	{
		class JIdentifier;
	}
	namespace Editor
	{
		class JEditorTreeStructure
		{
		private:
			std::set<GuiID> loadedOpendNodeID;
			bool applyLoadedNode = false;
		public:
			void Begin();
			void End();
		public:  
			bool DisplayTreeNode(const std::string& label, J_GUI_TREE_NODE_FLAG_ flags, const bool isFocus, const bool isActivated, const bool isSelected);
			bool CheckTreeNodeIsOpen(const std::string& label, J_GUI_TREE_NODE_FLAG_ flags, const bool isFocus, const bool isActivated, const bool isSelected);
		public:
			void TreePop();
		public:
			void LoadData(JFileIOTool& tool);
			void StoreData(JFileIOTool& tool, const GuiID wndID);
		public:
			J_GUI_TREE_NODE_FLAG_ GetBaseFlag()const noexcept;
		};
	}
}