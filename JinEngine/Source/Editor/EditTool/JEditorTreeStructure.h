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