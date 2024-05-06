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