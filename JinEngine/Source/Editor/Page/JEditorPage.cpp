#include"JEditorPage.h" 
#include"JEditorWindow.h"
#include"JEditorAttribute.h" 
#include"JEditorPageShareData.h"  
#include"../Popup/JEditorPopupWindow.h"
#include"../Event/JEditorEventStruct.h"
#include"../Event/JEditorEvent.h" 
#include"../../Core/Reflection/JReflectionInfo.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Object/Resource/Scene/JSceneManager.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/Resource/JResourceObject.h"
#include"../../Object/Resource/JResourceManager.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/JObjectFileIOHelper.h" 
#include"../Gui/JGui.h" 
#include<fstream> 

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			static constexpr int previousSizeExeFrame = 1;


		}

		JEditorPage::WindowInitInfo::WindowInitInfo(const std::string name)
			:name(name)
		{}
		std::string JEditorPage::WindowInitInfo::GetName()const noexcept
		{
			return name;
		}
		std::unique_ptr<JEditorAttribute> JEditorPage::WindowInitInfo::MakeAttribute()noexcept
		{
			return std::make_unique<JEditorAttribute>();
		}

		JEditorPage::JEditorPage(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_FLAG pageFlag)
			:JEditor(name + "##", std::move(attribute)), pageFlag(pageFlag)
		{
			auto openEditorWindowLam = [](JEditorPage& page, const std::string windowName)
			{
				JEditorWindow* selectedWindow = page.FindEditorWindow(windowName);
				if (selectedWindow->IsOpen())
					return;

				page.AddEventNotification(*JEditorEvent::EvInterface(), page.GetGuid(), J_EDITOR_EVENT::ACTIVATE_WINDOW,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(selectedWindow)));
			};
			auto closeEditorWindowLam = [](JEditorPage& page, const std::string windowName)
			{
				JEditorWindow* selectedWindow = page.FindEditorWindow(windowName);
				if (!selectedWindow->IsOpen())
					return;

				JGuiWindowInfo selectedWndInfo;
				JGui::GetWindowInfo(selectedWindow->GetName(), selectedWndInfo);
				bool canClose = false;
				if (!selectedWndInfo.hasDockNode)
					canClose = true;
				else
				{
					uint dockCount = 0;
					const uint wndCount = (uint)page.opendWindow.size();
					for (uint i = 0; i < wndCount; ++i)
					{
						JGuiWindowInfo opendWndInfo;
						JGui::GetWindowInfo(page.opendWindow[i]->GetName(), opendWndInfo);
						if (opendWndInfo.hasDockNode)
							++dockCount;
					}
					if (dockCount > 1)
						canClose = true;
				}
				if (canClose)
				{
					page.AddEventNotification(*JEditorEvent::EvInterface(), page.GetGuid(), J_EDITOR_EVENT::CLOSE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorCloseWindowEvStruct>(selectedWindow->GetName(), page.GetPageType())));
				}
				else
					*selectedWindow->GetOpenPtr() = true;
			};
 
			openEditorWindowFunctor = std::make_unique<OpenEditorWindowF::Functor>(openEditorWindowLam); 
			closeEditorWindowFunctor = std::make_unique<CloseEditorWindowF::Functor>(closeEditorWindowLam); 
		}
		JEditorPage::~JEditorPage()
		{}
		void JEditorPage::AddWindow(const std::vector<JEditorWindow*>& wnd)noexcept
		{
			windows = wnd;
			for (const auto& data : windows)
				windowMap.emplace(data->GetName(), data);
		}
		void JEditorPage::AddPopupWindow(const std::vector<JEditorPopupWindow*>& wnd)noexcept
		{
			popupWindow = wnd;
			closeConfirmPopupWindow = FindEditorPopupWindow(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM);
		} 
		J_EDITOR_PAGE_FLAG JEditorPage::GetPageFlag()const noexcept
		{
			return pageFlag;
		}
		bool JEditorPage::GetPublicState(const J_EDITOR_PAGE_PUBLIC_STATE stateType)const noexcept
		{ 
			switch (stateType)
			{
			case JinEngine::Editor::J_EDITOR_PAGE_PUBLIC_STATE::INPUT_LOCK:
				return state.isInputLock;
			case JinEngine::Editor::J_EDITOR_PAGE_PUBLIC_STATE::MAXIMIZE:
				return state.isMaximize;
			case JinEngine::Editor::J_EDITOR_PAGE_PUBLIC_STATE::MINIMIZE:
				return state.isMinimize;
			default:
				return false;
			}
		}
		uint JEditorPage::GetOpenWindowCount()const noexcept
		{
			return (uint)opendWindow.size();
		}
		JEditorWindow* JEditorPage::GetOpenWindow(const uint index)const noexcept
		{
			return opendWindow[index];
		}
		JEditorPopupWindow* JEditorPage::GetOpenPopupWindow()const noexcept
		{
			return opendPopupWindow;
		}
		std::vector<JEditorWindow*> JEditorPage::GetWindowVec()const noexcept
		{
			return windows;
		}
		GuiID JEditorPage::GetWindowID()const noexcept
		{
			JGuiWindowInfo info;
			JGui::GetWindowInfo(GetName(), info);
			return info.windowID;
		}
		GuiID JEditorPage::GetDockSpaceID()const noexcept
		{
			JGuiDockNodeInfo info;
			JGui::GetDockNodeInfo(GetDockNodeName(), info);
			return info.dockID;
		}
		JEditorPage::OpenEditorWindowF::Functor* JEditorPage::GetOpenEditorWindowFunctorPtr()noexcept
		{
			return openEditorWindowFunctor.get();
		}
		JEditorPage::CloseEditorWindowF::Functor* JEditorPage::GetCloseEditorWindowFunctorPtr()noexcept
		{
			return closeEditorWindowFunctor.get();
		}
		std::unique_ptr<JEditorPage::RequestOpenEditorWindowF::Functor> JEditorPage::GetRequestOpenEditorWindowFunctorPtr()noexcept
		{
			auto requestOpenEditorWindowLam = [](JEditorPage& page, const std::string windowName){page.RequestOpenWindow(windowName);};
			return std::make_unique<JEditorPage::RequestOpenEditorWindowF::Functor>(requestOpenEditorWindowLam);
		}
		std::unique_ptr<JEditorPage::RequestCloseEditorWindowF::Functor> JEditorPage::GetRequestCloseEditorWindowFunctorPtr()noexcept
		{
			auto requestCloseEditorWindowLam = [](JEditorPage& page, const std::string windowName){page.RequestCloseWindow(windowName);};
			return std::make_unique<JEditorPage::RequestCloseEditorWindowF::Functor>(requestCloseEditorWindowLam);
		}
		void JEditorPage::SetPageFlag(const J_EDITOR_PAGE_FLAG flag)noexcept
		{
			pageFlag = flag;
		}
		void JEditorPage::SetNextPagePos(const JVector2F& pos)noexcept
		{
			state.nextPos = pos;
			state.hasSetNextPosReq = true;
		}
		void JEditorPage::SetNextPageSize(const JVector2F& size)noexcept
		{
			state.nextSize = size;
			state.hasSetNextSizeReq = true;
		}
		void JEditorPage::SetInputLock(const bool value)noexcept
		{
			state.isInputLock = value;
		}
		void JEditorPage::SetMaximize(const bool value)noexcept
		{
			if (CanMaximize())
				state.isMaximize = value;
		}
		void JEditorPage::SetMinimize(const bool value)
		{
			if (CanMinimize())
				state.isMinimize = value;
		}
		bool JEditorPage::HasWindow(const std::string& name)noexcept
		{
			return FindEditorWindow(name) != nullptr;
		}
		bool JEditorPage::CanUpdate(JEditorWindow* wnd)const noexcept
		{
			if (wnd == nullptr)
				return false;

			//maximized window can update and
			//if previous size reserved other windows can be updated
			if (HasMaximizedWindow() && maximizeInfo->destroyAfFrame == -1 && maximizeInfo->window->GetGuid() != wnd->GetGuid())
				return false;

			return true;
		}
		bool JEditorPage::CanClose()const noexcept
		{
			return Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING);
		}
		bool JEditorPage::CanMaximize()const noexcept
		{
			return Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_WINDOW_MAXIMIZE);
		}
		bool JEditorPage::CanMinimize()const noexcept
		{
			return Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_WINDOW_MINIMIZE);
		}
		bool JEditorPage::HasDockNodeSpace()const noexcept
		{
			return JGui::HasDockNode(GetDockNodeName());
		}
		bool JEditorPage::HasMaximizedWindow()const noexcept
		{
			return maximizeInfo != nullptr;
		}
		void JEditorPage::Initialize()
		{
			JEditorPageShareData::PageInitData initData;
			initData.pageType = GetPageType();

			JEditorPageShareData::GetGuiIDF::CPtr getWindowID = &JEditorPage::GetWindowID;
			JEditorPageShareData::GetDockSpaceIDF::CPtr getDockSpaceID = &JEditorPage::GetDockSpaceID;
			JEditorPageShareData::GetPageNameF::CPtr getNamePtr = &JEditorPage::GetName;
			JEditorPageShareData::GetPageNameF::CPtr getDockSpaceNamePtr = &JEditorPage::GetDockNodeName;
			JEditorPageShareData::GetPageFlagF::CPtr getFlagPtr = &JEditorPage::GetPageFlag;
			JEditorPageShareData::GetPublicBooleanStateF::CPtr getPublicStatePtr = &JEditorPage::GetPublicState;

			initData.getGuiIDF = std::make_unique<JEditorPageShareData::GetGuiIDF::Functor>(getWindowID, this);
			initData.getDockSpaceIDF = std::make_unique<JEditorPageShareData::GetDockSpaceIDF::Functor>(getDockSpaceID, this);
			initData.getNameF = std::make_unique<JEditorPageShareData::GetPageNameF::Functor>(getNamePtr, this);
			initData.getDockSpaceNameF = std::make_unique<JEditorPageShareData::GetPageNameF::Functor>(getDockSpaceNamePtr, this);
			initData.getFlagF = std::make_unique<JEditorPageShareData::GetPageFlagF::Functor>(getFlagPtr, this);
			initData.getPublicStateF = std::make_unique<JEditorPageShareData::GetPublicBooleanStateF::Functor>(getPublicStatePtr, this);
			JEditorPageShareData::RegisterPage(initData);
		}
		void JEditorPage::Clear()
		{
			JEditorPageShareData::UnRegisterPage(GetPageType());
		}
		void JEditorPage::EnterPage(J_GUI_WINDOW_FLAG_ guiWindowFlag)noexcept
		{
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_DOCK))
			{
				J_GUI_DOCK_NODE_FLAG_ flag = J_GUI_DOCK_NODE_FLAG_NO_OVER_ME |
					J_GUI_DOCK_NODE_FLAG_NO_OVER_OTHER |
					J_GUI_DOCK_NODE_FLAG_NO_OVER_EMPTY | 
					J_GUI_DOCK_NODE_FLAG_NO_SPLIT_ME;
				if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_IS_MAIN_PAGE))
				{
					flag |= J_GUI_DOCK_NODE_FLAG_NO_SPLIT_OTHER;
					JGui::PushStyle(J_GUI_STYLE::WINDOW_ROUNDING, 0.0f);
					JGui::PushStyle(J_GUI_STYLE::WINDOW_PADDING, JVector2F(0.0f, 0.0f)); 
				}
				JGui::OverrideNextDockNodeFlag(flag);
			}

			if (state.isInputLock)
			{
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_DISABLED, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_NO_NAV, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_READ_ONLY, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_INPUTABLE, false);
				JGui::SetAllColorToSoft(JVector4F(0.2f, 0.2f, 0.2f, 0));
			}

			if (state.isMaximize)
			{
				JGui::SetNextWindowPos(JGui::GetMainWorkPos());
				JGui::SetNextWindowSize(JGui::GetMainWorkSize());
				guiWindowFlag = Core::AddSQValueEnum((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_MOVE, J_GUI_WINDOW_FLAG_NO_RESIZE);
			}
			if (state.isMinimize)
			{
				JGui::PushStyle(J_GUI_STYLE::WINDOW_MIN_SIZE, JVector2F(JGui::GetDefaultClientWindowMinSize().x, JGui::CalCloseButtionSize()));
				guiWindowFlag = Core::AddSQValueEnum((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_RESIZE);
			}

			if (state.hasSetNextPosReq)
			{
				JGui::SetNextWindowPos(state.nextPos);
				state.hasSetNextPosReq = false;
			}
			if (state.hasSetNextSizeReq)
			{
				JGui::SetNextWindowSize(state.nextSize);
				state.hasSetNextSizeReq = false;
			}

			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING))
			{
				bool res = false;
				res = JGui::BeginWindow(GetName(), &state.isPageOpen, guiWindowFlag);
				if (!state.isPageOpen)
				{
					if (closeConfirmPopupWindow == nullptr || closeConfirmPopupWindow->IsIgnoreConfirm())
					{
						if (state.isMaximize || state.isMinimize)
						{
							AddEventNotification(*JEditorEvent::EvInterface(),
								GetGuid(),
								J_EDITOR_EVENT::PREVIOUS_SIZE_PAGE,
								JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPreviousSizePageEvStruct>(this, false)));
						}
						RequestClosePage(JEditorClosePageEvStruct(GetPageType()), IsActivated());
					}
					else
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::OPEN_POPUP_WINDOW,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPopupWindowEvStruct>(closeConfirmPopupWindow, GetPageType())));
					}
				}
			}
			else
				JGui::BeginWindow(GetName().c_str(), nullptr, guiWindowFlag);

			if (CanMinimize())
			{
				if (state.isMinimize)
				{
					if (JGui::PreviousSizeButton(CanClose(), CanMaximize() && state.isMinimize))
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PREVIOUS_SIZE_PAGE,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPreviousSizePageEvStruct>(this, true)));
					}
				}
				else if (JGui::MinimizeButton(CanClose(), CanMaximize()))
				{
					const float minHeight = JGui::GetWindowTitleBarSize().y * 0.5f;
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::MINIMIZE_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorMinimizePageEvStruct>(this, JGui::GetWindowPos(), JGui::GetWindowSize(), minHeight)));
				}
			}
			if (CanMaximize())
			{
				if (state.isMaximize)
				{
					if (JGui::PreviousSizeButton(CanClose(), false))
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PREVIOUS_SIZE_PAGE,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPreviousSizePageEvStruct>(this, true)));
					}
				}
				else if (JGui::MaximizeButton())
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::MAXIMIZE_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorMaximizePageEvStruct>(this, JGui::GetWindowPos(), JGui::GetWindowSize())));
				}
			}
		}
		void JEditorPage::ClosePage()noexcept
		{
			JGui::EndWindow();
			SetLastActivated(IsActivated());

			if (state.isMinimize)
				JGui::PopStyle(1);

			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_DOCK) &&
				Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_IS_MAIN_PAGE))
				JGui::PopStyle(2);

			if (state.isInputLock)
			{
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::SetAllColorToSoft(JVector4F(-0.2f, -0.2f, -0.2f, 0));
			}
		}
		void JEditorPage::UpdateOpenPopupWindow(const JVector2<float>& pagePos, const JVector2<float>& pageSize)
		{
			JEditorPopupWindow* openPopup = GetOpenPopupWindow();
			if (openPopup != nullptr)
				openPopup->Update(GetName(), pagePos, pageSize);
		}
		void JEditorPage::UpdateOpenWindow()
		{
			for (const auto& data : opendWindow)
			{
				if (CanUpdate(data))
					data->UpdateWindow();
			}
			UpdateMaximizeLife();
		} 
		void JEditorPage::UpdateDockSpace(J_GUI_DOCK_NODE_FLAG_ dockspaceFlag)
		{
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_SUPPORT_DOCK))
				JGui::UpdateDockSpace(GetDockNodeName(), JGui::GetMainWorkSize(), dockspaceFlag);
		}
		void JEditorPage::UpdateMaximizeLife()
		{
			if (maximizeInfo != nullptr && maximizeInfo->destroyAfFrame != -1)
			{
				if (maximizeInfo->destroyAfFrame != 0)
					--maximizeInfo->destroyAfFrame;
				else
				{
					//end maximize
					//restore before state
					JGuiWindowInfo wndInfo;
					JGui::GetWindowInfo(maximizeInfo->window->GetName(), wndInfo);
					JGui::FocusWindow(wndInfo.windowID);
					JGui::RestoreFromMaximize(wndInfo.windowID, maximizeInfo->preTabItemID);
					maximizeInfo = nullptr;
				}
			}
		}
		void JEditorPage::OpenWindow(const std::string& windowname)noexcept
		{
			OpenWindow(FindEditorWindow(windowname));
		}
		void JEditorPage::OpenWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			bool hasWindow = IsWindowInVector(window, windows.cbegin(), (uint)windows.size());
			if (!hasWindow)
				return;

			opendWindow.push_back(window);
			window->SetOpen();
		}
		void JEditorPage::CloseWindow(const std::string& windowname)noexcept
		{
			CloseWindow(FindEditorWindow(windowname));
		}
		void JEditorPage::CloseWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (window->IsActivated())
				DeActivateWindow(window);

			window->SetClose();
			opendWindow.erase(opendWindow.begin() + windowIndex);
		}
		void JEditorPage::ActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr || !IsActivated())
				return;

			if (!window->IsOpen())
				OpenWindow(window);

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->Activate();
		}
		void JEditorPage::DeActivateWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr || !IsActivated())
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (window->IsFocus())
				UnFocusWindow(window);
			window->DeActivate();
		}
		void JEditorPage::FocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			//simple window는 JEditorPage에서 focusWindow 대상이 아니지만 gui내부에서는 focus됨으로
			//JEditorWindow -> SimpleWindow -> JEditorWindow순으로 Focus시
			//JEditorWindow -> JEditorWindow 순으로 JEditorPage는 포커싱 대상을 변경하므로
			//같은 window가 포커싱 처리과정을 중복 수행할수있으므로 이를 방지.
			if (focusWindow != nullptr && focusWindow->GetGuid() == window->GetGuid())
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			if (!window->IsActivated())
				ActivateWindow(window);
			window->SetFocus();
			if (focusWindow != nullptr)
				UnFocusWindow(focusWindow);
			focusWindow = window;  

			const uint count = (uint)opendWindow.size() - 1;
			for (uint i = windowIndex; i < count; ++i)
				opendWindow[i] = opendWindow[i + 1];
			opendWindow[count] = focusWindow;
		}
		void JEditorPage::FocusWindow(const std::string& windowName)noexcept
		{
			FocusWindow(FindEditorWindow(windowName));
		}
		void JEditorPage::UnFocusWindow(JEditorWindow* window)noexcept
		{
			if (window == nullptr)
				return;

			int windowIndex = FindWindowIndex(window, opendWindow.cbegin(), (uint)opendWindow.size());
			if (windowIndex == -1)
				return;

			window->SetUnFocus();
			focusWindow = nullptr;
		}
		void JEditorPage::MaximizeWindow(JEditorWindow* window, const JVector2F& prePos, const JVector2F& preSize)noexcept
		{
			if (window == nullptr || maximizeInfo != nullptr)
				return;

			maximizeInfo = std::make_unique<WinodowMaximizeInfo>();
			maximizeInfo->window = window;
			maximizeInfo->window->SetMaximize(true);
			maximizeInfo->prePos = prePos;
			maximizeInfo->preSize = preSize;
			//maximizeInfo->preWindowSize = 
			JGuiWindowInfo windowInfo;
			JGui::GetWindowInfo(maximizeInfo->window->GetName(), windowInfo);
			if (windowInfo.hasDockNode)
			{
				JGuiDockNodeInfo dockInfo;
				JGui::GetDockNodeInfoByWindowName(maximizeInfo->window->GetName(), dockInfo);
				maximizeInfo->preTabItemID = dockInfo.tabItemID;
			}
		}
		void JEditorPage::PreviousSizeWindow(JEditorWindow* window, const bool useLazy)noexcept
		{
			if (window == nullptr || maximizeInfo->window == nullptr || window->GetGuid() != maximizeInfo->window->GetGuid())
				return;

			maximizeInfo->window->SetNextWindowPos(maximizeInfo->prePos);
			maximizeInfo->window->SetNextWindowSize(maximizeInfo->preSize);
			maximizeInfo->window->SetMaximize(false);
			if (useLazy)
				maximizeInfo->destroyAfFrame = previousSizeExeFrame;
			else
			{
				maximizeInfo->destroyAfFrame = 0;
				UpdateMaximizeLife();
			}
		}
		void JEditorPage::OpenPopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType,
			const std::string& desc,
			std::vector<PopupWndFuncTuple>&& tupleVec)
		{
			OpenPopupWindow(FindEditorPopupWindow(popupType), desc, std::move(tupleVec));
		}
		void JEditorPage::OpenPopupWindow(JEditorPopupWindow* popupWindow,
			const std::string& desc,
			std::vector<PopupWndFuncTuple>&& tupleVec)
		{
			if (popupWindow != nullptr && !popupWindow->IsOpen())
			{
				//SetPageFlag(Core::AddSQValueEnum(GetPageFlag(), J_EDITOR_PAGE_WINDOW_INPUT_LOCK));
				if (opendPopupWindow != nullptr)
					ClosePopupWindow(opendPopupWindow->GetPopupType());
				opendPopupWindow = popupWindow;
				opendPopupWindow->SetDesc(desc);
				const uint vecCount = (uint)tupleVec.size();
				for (uint i = 0; i < vecCount; ++i)
					opendPopupWindow->RegisterBind(std::get<0>(tupleVec[i]), std::move(std::get<1>(tupleVec[i])));
				opendPopupWindow->SetOpen();
			}
		}
		void JEditorPage::ClosePopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType)
		{
			ClosePopupWindow(FindEditorPopupWindow(popupType));
		}
		void JEditorPage::ClosePopupWindow(JEditorPopupWindow* popupWindow)
		{
			if (popupWindow != nullptr && popupWindow->IsOpen())
			{
				//SetPageFlag(Core::MinusSQValueEnum(GetPageFlag(), J_EDITOR_PAGE_WINDOW_INPUT_LOCK));
				popupWindow->SetClose();
				opendPopupWindow = nullptr;
			}
		}
		void JEditorPage::PrintOpenWindowState()
		{
			const uint openWindowCount = (uint)opendWindow.size();
			JGui::SetCursorPos(JVector2F(50, 50));
			JGui::BeginWindow("PrintOpenWindowState");
			for (uint i = 0; i < openWindowCount; ++i)
			{
				JGui::Text(opendWindow[i]->GetName());
				if (opendWindow[i]->IsOpen())
					JGui::Text("Open");
				else
					JGui::Text("Close");

				if (opendWindow[i]->IsActivated())
					JGui::Text("Activate");
				else
					JGui::Text("DeActivate");

				if (opendWindow[i]->IsFocus())
					JGui::Text("Focus On");
				else
					JGui::Text("Focus Off");
			}
			JGui::EndWindow();
		}
		void JEditorPage::RequestOpenWindow(std::string windowName)
		{
			JEditorWindow* selectedWindow = FindEditorWindow(windowName);
			if (selectedWindow ==nullptr || selectedWindow->IsOpen())
				return;

			std::string taskName = "Open window";
			std::string taskDesc = "window name: " + windowName;
 
			auto doBinder = std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpenEditorWindowFunctorPtr(), *this, std::string(windowName));
			auto undoBinder = std::make_unique<CloseEditorWindowF::CompletelyBind>(*GetCloseEditorWindowFunctorPtr(), *this, std::string(windowName));
			auto task = std::make_unique<Core::JTransitionSetValueTask>(taskName, taskDesc, std::move(doBinder), std::move(undoBinder));
  
			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditorPage::RequestCloseWindow(std::string windowName)
		{
			JEditorWindow* selectedWindow = FindEditorWindow(windowName);
			if (selectedWindow == nullptr || !selectedWindow->IsOpen())
				return;

			std::string taskName = "Close window";
			std::string taskDesc = "window name: " + windowName;

			auto doBinder = std::make_unique<CloseEditorWindowF::CompletelyBind>(*GetCloseEditorWindowFunctorPtr(), *this, std::string(windowName));
			auto undoBinder = std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpenEditorWindowFunctorPtr(), *this, std::string(windowName));
			auto task = std::make_unique<Core::JTransitionSetValueTask>(taskName, taskDesc, std::move(doBinder), std::move(undoBinder));

			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditorPage::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
			state.isPageOpen = true;
		}
		void JEditorPage::DoSetClose()noexcept
		{
			JEditorPageShareData::ClearPageData(GetPageType());
			state.isPageOpen = false;
			JEditor::DoSetClose();
		}
		void JEditorPage::DoActivate()noexcept
		{
			JEditor::DoActivate();
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
			{
				if (opendWindow[i]->IsLastActivated())
					opendWindow[i]->Activate();
			}
		}
		void JEditorPage::DoDeActivate()noexcept
		{
			const uint opendWindowCount = (uint)opendWindow.size();
			for (uint i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->DeActivate();
			JEditor::DoDeActivate();
		}
		JEditorWindow* JEditorPage::FindEditorWindow(const std::string& windowName)const noexcept
		{
			auto data = windowMap.find(windowName);
			if (data != windowMap.end())
				return data->second;
			else
				return nullptr; 
		}
		JEditorPopupWindow* JEditorPage::FindEditorPopupWindow(const J_EDITOR_POPUP_WINDOW_TYPE popupType)const noexcept
		{
			const uint windowCount = (uint)popupWindow.size();
			for (uint i = 0; i < windowCount; ++i)
			{
				if (popupWindow[i]->GetPopupType() == popupType)
					return popupWindow[i];
			}
			return nullptr;
		}
		bool JEditorPage::IsWindowInVector(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept
		{
			const size_t tarGuid = window->GetGuid();
			for (uint i = 0; i < count; ++i)
			{
				if ((*(st + i))->GetGuid() == tarGuid)
					return true;
			}
			return false;
		}
		int JEditorPage::FindWindowIndex(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept
		{
			const size_t tarGuid = window->GetGuid();
			for (uint i = 0; i < count; ++i)
			{
				if ((*(st + i))->GetGuid() == tarGuid)
					return i;
			}
			return -1;
		}
		void JEditorPage::StorePage(JFileIOTool& tool)
		{
			JObjectFileIOHelper::StoreJString(tool, GetName(), "PageName:");
			JObjectFileIOHelper::StoreAtomicData(tool, GetGuid(), "PageGuid:");
			JObjectFileIOHelper::StoreAtomicData(tool, IsOpen(), "Open:");
			JObjectFileIOHelper::StoreAtomicData(tool, IsActivated(), "Activate:");
			JObjectFileIOHelper::StoreAtomicData(tool, IsFocus(), "Focus:");
			JObjectFileIOHelper::_StoreHasIden(tool, JEditorPageShareData::GetOpendPageData(GetPageType()).GetOpenSeleted().Get(), "OpenSelected");
			
			bool hasFocusWindow = focusWindow != nullptr;
			JObjectFileIOHelper::StoreAtomicData(tool, hasFocusWindow, "HasFocusWindow:");
			if (hasFocusWindow)
				JObjectFileIOHelper::StoreJString(tool, focusWindow->GetName(), "FocusWindowName:");
			else
				JObjectFileIOHelper::StoreJString(tool, "None", "FocusWindowName:");

			JObjectFileIOHelper::StoreAtomicData(tool, (uint)windows.size(), "WindowCount");
			const uint editorWindowCount = (uint)windows.size();

			tool.PushArrayOwner("WindowData");
			for (uint i = 0; i < editorWindowCount; ++i)
			{
				tool.PushArrayMember();
				windows[i]->StoreEditorWindow(tool);
				tool.PopStack();
			}
			tool.PopStack();
		}
		void JEditorPage::LoadPage(JFileIOTool& tool)
		{
			std::wstring guide;
			std::wstring name;
			size_t pageGuid;
			bool isOpen;
			bool active;
			bool isFocus;
			JUserPtr<Core::JIdentifier> openObj = nullptr;
			bool hasFocusWindow;
			std::wstring focusWindowName;
			int windowCount;

			JObjectFileIOHelper::LoadJString(tool, name, "PageName:");
			JObjectFileIOHelper::LoadAtomicData(tool, pageGuid, "PageGuid:");
			JObjectFileIOHelper::LoadAtomicData(tool, isOpen, "Open:");
			JObjectFileIOHelper::LoadAtomicData(tool, active, "Activate:");
			JObjectFileIOHelper::LoadAtomicData(tool, isFocus, "Focus:");
			 
			openObj = JObjectFileIOHelper::_LoadHasIden(tool, "OpenSelected");
			JObjectFileIOHelper::LoadAtomicData(tool, hasFocusWindow, "HasFocusWindow:");
			JObjectFileIOHelper::LoadJString(tool, focusWindowName, "FocusWindowName:");
			JObjectFileIOHelper::LoadAtomicData(tool, windowCount, "WindowCount");

			if (isOpen)
			{
				bool alreadyHasValidOpenObj = JEditorPageShareData::HasValidOpenPageData(GetPageType());
				bool hasOpenObj = openObj != nullptr && openObj->GetTypeInfo().IsChildOf(JObject::StaticTypeInfo());
				bool canOpen = alreadyHasValidOpenObj || hasOpenObj;
				if (alreadyHasValidOpenObj)
				{
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::OPEN_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(GetPageType(), JUserPtr<JObject>())));
				}
				else if (hasOpenObj)
				{
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::OPEN_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(GetPageType(), openObj)));
				}
				if (canOpen && active)
				{
					AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::ACTIVATE_PAGE,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(GetPageType())));

					if (isFocus)
					{
						AddEventNotification(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::FOCUS_PAGE,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorFocusPageEvStruct>(this)));
					}
				}
			}
			tool.PushExistStack("WindowData");
			for (uint i = 0; i < windowCount; ++i)
			{
				tool.PushExistStack();
				windows[i]->LoadEditorWindow(tool);
				tool.PopStack();
			}
			tool.PopStack();
		}
	}
}