#include"JLogViewer.h"
#include"../../JEditorAttribute.h"  
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../Gui/JGui.h"
#include"../../../../Core/Log/JLog.h"
#include"../../../../Core/Log/JLogHandler.h"
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Core/Utility/JCommonUtility.h"
#include"../../../../Develop/Debug/JDevelopDebug.h"
#include"../../../../Develop/Debug/JDevelopDebugConstants.h"
#include<deque>

namespace JinEngine
{
	namespace Editor
	{ 
		namespace
		{ 
			static std::vector<Core::JLogBase*> GetUserLog()
			{
				return std::vector<Core::JLogBase*>{};
			}
			static std::vector<Core::JLogBase*> GetTransitionLog()
			{
				return Core::JPublicLogHolder::GetLogVec(JEditorTransition::Name());
			}
			static std::vector<Core::JLogBase*> GetDevelopLog()
			{
				//Develop::JDevelopDebug::
				return Core::JPublicLogHolder::GetLogVec(Develop::Constants::defualtLogHandlerName);
			}
			static std::vector<Core::JLogBase*> GetAllLog()
			{
				std::vector<Core::JLogBase*> user = GetUserLog();
				std::vector<Core::JLogBase*> trans = GetTransitionLog();
				std::vector<Core::JLogBase*> develop = GetDevelopLog();
				user.insert(user.end(), trans.begin(), trans.end());
				user.insert(user.end(), develop.begin(), develop.end());
				return user;
			} 
			static void ClearUserLog()
			{

			}
			static void ClearTransitionLog()
			{
				Core::JPublicLogHolder::ClearBuffer(JEditorTransition::Name());
			}
			static void ClearDevelopLog()
			{
				Core::JPublicLogHolder::ClearBuffer(Develop::Constants::defualtLogHandlerName);
			}
			static void ClearAllLog()
			{
				ClearUserLog();
				ClearTransitionLog();
				ClearDevelopLog();
			}
		}
		JLogViewer::JLogViewer(const std::string& name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag)
		{
			//All Log	User, Transition All, Task
			std::string itemlabel[tabItemCount]
			{
				"All Log", "User", "Editor Task", "Develop"
			};
			tabBarHelper = std::make_unique<JEditorTabBarHelper<tabItemCount>>(itemlabel);

			getLogVecPtr[0] = &GetAllLog;
			getLogVecPtr[1] = &GetUserLog;
			getLogVecPtr[2] = &GetTransitionLog;
			getLogVecPtr[3] = &GetDevelopLog;
			 
			clearLogHandlerPtr[0] = &ClearAllLog;
			clearLogHandlerPtr[1] = &ClearUserLog;
			clearLogHandlerPtr[2] = &ClearTransitionLog;
			clearLogHandlerPtr[3] = &ClearDevelopLog;		 
		}
		J_EDITOR_WINDOW_TYPE JLogViewer::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::LOG_VIEWER;
		}
		void JLogViewer::Initialize()noexcept
		{}
		void JLogViewer::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
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
			if (JGui::BeginTabBar("##JLogViewerTab"))
			{
				//JGui::SetCursorPosX(JGui::GetCursorPosX() + xPadding);
				for (int i = 0; i < tabItemCount; ++i)
				{
					if (JGui::BeginTabItem(tabBarHelper->GetLabel(i)))
					{
						tabBarHelper->OpenItemBit(i); 
						JGui::EndTabItem();
					}
				}
				JGui::EndTabBar();
			}
			 
			if(tabBarHelper->IsOpenNewTab())
				selectedLogIndex = invalidIndex;
			  
			const JVector4<float> wndBgCol = JGui::GetColor( J_GUI_COLOR::WINDOW_BG);
			const JVector4<float> wndBgSotfCol = wndBgCol + JVector4<float>(0.15f, 0.15f, 0.15f, 0.15f);

			JGui::PushColor(J_GUI_COLOR::FRAME_BG, JGui::GetColor(J_GUI_COLOR::WINDOW_BG)); 
			JVector2<float> nowCursorPos = JGui::GetCursorPos();
			//JVector2<float> restSize = JGui::GetRestWindowSpace();
			JVector2<float> contentsSize = JGui::GetRestWindowContentsSize();
			JVector2<float> titleSize = JVector2<float>(contentsSize.x, JGui::GetAlphabetSize().y * 1.25f);
			JVector2<float> buttonSize = JGui::CalButtionSize("Clear##JLogViewerLogList#", JVector2F::Zero());
			float offsetY = JGui::GetFrameBorderSize() + JGui::GetFramePadding().y;
			//JGui::SetCursorPos(JGui::GetCursorPos() + restSize * 0.1f);
		 
			JEditorTextAlignCalculator textAlign; 
			std::string seletableStr = "";

			JGui::SetCurrentWindowFontScale(0.8f);
			if (JGui::BeginListBox("##JLogViewerLogList", JVector2<float>(contentsSize.x, contentsSize.y - buttonSize.y - offsetY)))
			{
				if (tabBarHelper->IsActivatedItem(0))
					selectedTabIndex = 0;
				else if (tabBarHelper->IsActivatedItem(1))
					selectedTabIndex = 1; 
				else if (tabBarHelper->IsActivatedItem(2))
					selectedTabIndex = 2;
				else if (tabBarHelper->IsActivatedItem(3))
					selectedTabIndex = 3;
								
				std::vector<Core::JLogBase*> logVec= getLogVecPtr[selectedTabIndex]();
				auto logCompareLam = [](Core::JLogBase* a, Core::JLogBase* b) {return *a < *b; };
				std::sort(logVec.begin(), logVec.end(), logCompareLam);

				//logVec 갯수가 변경됬을경우(clear, pop)
				if (logVec.size() <= selectedLogIndex)
					selectedLogIndex = invalidIndex;

				JGui::Separator();
				const uint vecCount = (uint)logVec.size();
				const float titleRange = contentsSize.x;
				  
				for (uint i = 0; i < vecCount; ++i)
				{
					std::string title = logVec[i]->GetTitle();
					if(title.empty())
						title = logVec[i]->GetBody();
					 
					textAlign.Update(title, titleSize, true);
					if (JGui::Selectable(textAlign.LeftAligned() + "##" + std::to_string(i) + "JLogViewer"))
						selectedLogIndex = i;

					if (selectedLogIndex != invalidIndex)
						seletableStr = logVec[selectedLogIndex]->GetLog() + "\n time: " + logVec[selectedLogIndex]->GetTime().ToString();
					JGui::Separator();
				}
				JGui::Separator();
				if (selectedLogIndex != invalidIndex)
				{
					JGui::PushTreeNodeColorSet(true, true, true);
					textAlign.Update(seletableStr, titleSize, false);
					JGui::Selectable(textAlign.LeftAligned() + "##JLogViewerSelected");
					JGui::PopTreeNodeColorSet(true, true);
				}
				JGui::EndListBox();
			}
			if (JGui::Button("Clear##JLogViewerLogList#"))
			{
				clearLogHandlerPtr[selectedTabIndex]();
				selectedLogIndex = invalidIndex;
			}
			bool isDefaultLogActivated = Develop::JDevelopDebug::IsActivate();
			if (!isDefaultLogActivated)
				JGui::PushButtonColorDeActSet();

			JGui::SameLine();
			if (JGui::Button("Default Log##JLogViewerLogList#"))
			{
				if(isDefaultLogActivated)
					Develop::JDevelopDebug::DeActivate();
				else
					Develop::JDevelopDebug::Activate();
			}
			if (!isDefaultLogActivated)
				JGui::PopButtonColorDeActSet();

			//JGui::SetCursorPosY(nowCursorPos.y + restSize.y * 0.75f);
			JGui::SetCurrentWindowFontScale(1);
			JGui::PopColor();
		}
		void JLogViewer::DoActivate()noexcept
		{
			JEditorWindow::DoActivate();
			selectedTabIndex = 0;
			selectedLogIndex = 0;
			tabBarHelper->SetInitState();
		}
		void JLogViewer::DoDeActivate()noexcept
		{
			JEditorWindow::DoDeActivate();
		}
	}
}