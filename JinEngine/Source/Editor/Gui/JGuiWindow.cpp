#include"JGuiWindow.h"

namespace JinEngine
{
	namespace Editor
	{
		JGuiDockBuildNode::JGuiDockBuildNode(const std::string& windowName, const std::string& dockSpaceName)
			:windowName(windowName), dockSpaceName(dockSpaceName)
		{}
		JGuiDockBuildNode::JGuiDockBuildNode(const std::string& windowName,
			const int nodeNumber,
			const int parentNumber,
			const J_GUI_CARDINAL_DIR splitDir ,
			const float splitRate)
			:windowName(windowName), nodeNumber(nodeNumber), parentNumber(parentNumber), splitDir(splitDir), splitRate(splitRate)
		{}

		std::unique_ptr<JGuiDockBuildNode> JGuiDockBuildNode::CreateRootNode(const std::string& windowName, const std::string& dockSpaceName)
		{
			return std::make_unique<JGuiDockBuildNode>(windowName, dockSpaceName);
		}
		std::unique_ptr<JGuiDockBuildNode> JGuiDockBuildNode::CreateNode(const std::string& windowName,
			const int nodeNumber,
			const int parentNumber,
			const J_GUI_CARDINAL_DIR splitDir,
			const float splitRate)
		{
			return std::make_unique<JGuiDockBuildNode>(windowName, nodeNumber, parentNumber, splitDir, splitRate);
		}
	}
}