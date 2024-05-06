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
#include"JGuiType.h" 
#include"../../Core/Math/JVector.h" 

namespace JinEngine
{
	namespace Editor
	{ 
		class JGuiDockBuildNode
		{
		public:
			std::string windowName;
			//root node�� ��쿡�� ��ȿ
			std::string dockSpaceName;
		public:
			int nodeNumber;						//0 ~ n
			int parentNumber = invalidIndex;	//invalidIndex�ϰ�� root ���
		public:
			J_GUI_CARDINAL_DIR splitDir;
			float splitRate;
		public:  
			JGuiDockBuildNode(const std::string& windowName, const std::string& dockSpaceName);
			JGuiDockBuildNode(const std::string& windowName, 
				const int nodeNumber,
				const int parentNumber,
				const J_GUI_CARDINAL_DIR splitDir,
				const float splitRate);
		public:
			static std::unique_ptr<JGuiDockBuildNode> CreateRootNode(const std::string& windowName, const std::string& dockSpaceName);
			static std::unique_ptr<JGuiDockBuildNode> CreateNode(const std::string& windowName,
				const int nodeNumber,
				const int parentNumber,
				const J_GUI_CARDINAL_DIR splitDir,
				const float splitRate);
		};
		class JGuiDockNodeInfo
		{
		public:
			size_t dockID = 0;
			size_t parentDockID = 0; 
			size_t visibleWindowID = 0;
		public:
			size_t selectedTabID;
			std::vector<size_t> tabItemID;
		public:
			int order = -1;
		public:
			bool isRootNode = false;
			bool isFocused = false;
			bool isVisible = false;  
		}; 
		class JGuiWindowInfo
		{
		public:
			size_t windowID = 0;
			size_t dockID = 0;
			size_t lastDockID = 0;	//last valid dock id
			std::string windowName;
		public:
			JVector2F pos;
			JVector2F size;
		public:
			int focusOrder = -1;
			int dockOrder = -1;
			int dockTabItemCount = 0;
		public:
			bool isLastFrameFocused = false;
			bool isLastFrameActive = false; 
			bool hasDockNode = false;
		};
	}
}