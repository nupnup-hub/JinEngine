#include"JLogViewer.h"
#include"../../JEditorAttribute.h"  
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../../../Core/Log/JLog.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Utility/JCommonUtility.h"
#include<deque>

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
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
				"All Log", "User", "Editor Task"
			};
			tabBarHelper = std::make_unique<JEditorTabBarHelper<tabItemCount>>(itemlabel);
		}
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
			if (JImGuiImpl::BeginTabBar("AvatarSetting"))
			{
				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
				for (int i = 0; i < tabItemCount; ++i)
				{
					if (JImGuiImpl::BeginTabItem(tabBarHelper->GetLabel(i)))
					{
						tabBarHelper->OpenItemBit(i);
						JImGuiImpl::EndTabItem();
					}
				}
				JImGuiImpl::EndTabBar();
			}

			const JVector4<float> preFrameCol = JImGuiImpl::GetColor(ImGuiCol_FrameBg);
			const JVector4<float> wndBgCol = JImGuiImpl::GetColor(ImGuiCol_WindowBg);
			const JVector4<float> wndBgSotfCol = wndBgCol + JVector4<float>(0.15f, 0.15f, 0.15f, 0.15f);

			JImGuiImpl::SetColor(JImGuiImpl::GetColor(ImGuiCol_WindowBg), ImGuiCol_FrameBg);
			JVector2<float> nowCursorPos = ImGui::GetCursorPos();
			JVector2<float> restSize = JImGuiImpl::GetRestWindowSpace();
			//ImGui::SetCursorPos(ImGui::GetCursorPos() + restSize * 0.1f);

			static int seletabeIndex = -1;
			static std::string seletableStr = "";
			if (JImGuiImpl::BeginListBox("##JLogViewer_TransitionLog", JVector2<float>(restSize.x, restSize.y * 0.75f)))
			{
				std::vector<Core::JLogBase*> logVec; 
				if (tabBarHelper->IsActivatedItem(0))
					logVec = GetAllLog();
				else if (tabBarHelper->IsActivatedItem(1))
					logVec = GetUserLog();
				else if (tabBarHelper->IsActivatedItem(2))
					logVec = GetTransitionLog(); 

				auto logCompareLam = [](Core::JLogBase* a, Core::JLogBase* b) {return *a < *b; };
				std::sort(logVec.begin(), logVec.end(), logCompareLam);

				ImGui::Separator();
				const uint vecCount = (uint)logVec.size();
				for (uint i = 0; i < vecCount; ++i)
				{
					if (JImGuiImpl::Selectable(logVec[i]->GetTitle() + "##" + std::to_string(i) + "JLogViewer"))
						seletabeIndex = i;

					if (seletabeIndex != -1)
						seletableStr = logVec[seletabeIndex]->GetLog() + "\n time: " + logVec[seletabeIndex]->GetTime().ToString();
					ImGui::Separator();
				}
				JImGuiImpl::EndListBox();
			}
			ImGui::SetCursorPosY(nowCursorPos.y + restSize.y * 0.75f);
			if (seletabeIndex != -1)
				JImGuiImpl::Text(seletableStr);
			JImGuiImpl::SetColor(preFrameCol, ImGuiCol_FrameBg);
		}
		std::vector<Core::JLogBase*> JLogViewer::GetAllLog()
		{
			std::vector<Core::JLogBase*> user = GetUserLog();
			std::vector<Core::JLogBase*> trans = GetTransitionLog();
			user.insert(user.end(), trans.begin(), trans.end());
			return user;
		}
		std::vector<Core::JLogBase*> JLogViewer::GetUserLog()
		{
			return std::vector<Core::JLogBase*>{};
		}
		std::vector<Core::JLogBase*> JLogViewer::GetTransitionLog()
		{
			return JEditorTransition::Instance().GetLogHandler()->GetLogVec();
		}
	}
}