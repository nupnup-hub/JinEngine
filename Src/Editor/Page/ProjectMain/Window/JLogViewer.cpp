#include"JLogViewer.h"
#include"../../JEditorAttribute.h" 
#include"../../../Transition/JEditorTransition.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Utility/JCommonUtility.h"
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		JLogViewer::JLogViewer(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(std::move(attribute), ownerPageType)
		{}
		void JLogViewer::Initialize()noexcept
		{}
		void JLogViewer::UpdateWindow()
		{
			JEditorWindow::UpdateWindow();
			BuildLogViewer();
		}
		void JLogViewer::BuildLogViewer()
		{
			if (onSceneLog)
				SceneLogOnScreen();
			if (onEditorLog)
				EditorLogOnScreen();
		}
		void JLogViewer::SceneLogOnScreen()
		{
			//¹Ì±¸Çö
		}
		void JLogViewer::EditorLogOnScreen()
		{
			const std::deque<std::string>& editorLogQueue = JEditorTransition::GetLog();
			const uint queueCount = (uint)editorLogQueue.size();
			for (uint i = 0; i < queueCount; ++i)
				JImGuiImpl::Text(editorLogQueue[i]);
		}
	}
}