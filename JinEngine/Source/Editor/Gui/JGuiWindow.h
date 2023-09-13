#pragma once
#include"JGuiType.h" 
#include"../../Core/Math/JVector.h"
#include<string>
#include<memory>
#include<vector>

namespace JinEngine
{
	namespace Editor
	{ 
		class JGuiDockBuildNode
		{
		public:
			std::string windowName;
			//root node일 경우에만 유효
			std::string dockSpaceName;
		public:
			int nodeNumber;						//0 ~ n
			int parentNumber = invalidIndex;	//invalidIndex일경우 root 취급
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