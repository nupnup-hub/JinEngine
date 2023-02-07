#include"JLogViewer.h"
#include"../../JEditorAttribute.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Utility/JCommonUtility.h"
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		JLogViewer::JLogViewer(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{}
		J_EDITOR_WINDOW_TYPE JLogViewer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::LOG_VIEWER;
		}
		void JLogViewer::Initialize()noexcept
		{}
		void JLogViewer::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated())
			{ 
				UpdateMouseClick();
				BuildLogViewer();
			}
			CloseWindow();
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
			const std::deque<std::string>& editorLogQueue = Core::JTransition::GetLog();
			const uint queueCount = (uint)editorLogQueue.size();
			for (uint i = 0; i < queueCount; ++i)
				JImGuiImpl::Text(editorLogQueue[i]);
		}
	}
}