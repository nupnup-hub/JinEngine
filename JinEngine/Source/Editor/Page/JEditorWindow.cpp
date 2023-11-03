#include"JEditorWindow.h"
#include"JEditorPageShareData.h"
#include"JEditorAttribute.h"  
#include"Docking/JDockUpdateHelper.h"
#include"../Event/JEditorEvent.h" 
#include"../Popup/JEditorPopupMenu.h"
#include"../Gui/JGui.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Transition/JTransition.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectPrivate.h" 
#include"../../Window/JWindow.h"
#include<fstream>
 
namespace JinEngine
{
	namespace Editor
	{
		JEditorWindow::PopupSetting::PopupSetting(JEditorPopupMenu* popupMenu,
			JEditorStringMap* stringMap,
			const bool canOpenPopup,
			const bool focusWindowIfCloseThisFrame)
			:popupMenu(popupMenu), 
			stringMap(stringMap),
			canOpenPopup(canOpenPopup),
			focusWindowIfCloseThisFrame(focusWindowIfCloseThisFrame)
		{}
		bool JEditorWindow::PopupSetting::IsValid()const noexcept
		{
			return popupMenu != nullptr && stringMap != nullptr;
		}
		JEditorWindow::JEditorWindow(const std::string name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditor(name, std::move(attribute)), ownerPageType(ownerPageType), windowFlag(windowFlag)
		{

		}
		JEditorWindow::~JEditorWindow() {}
		J_EDITOR_PAGE_TYPE JEditorWindow::GetOwnerPageType()const noexcept
		{
			return ownerPageType;
		}
		void JEditorWindow::EnterWindow(J_GUI_WINDOW_FLAG_ guiWindowFlag)
		{
			J_EDITOR_PAGE_FLAG pageFlag = JEditorPageShareData::GetPageFlag(ownerPageType);
			const bool inputLocked = JEditorPageShareData::GetPublicState(ownerPageType, J_EDITOR_PAGE_PUBLIC_STATE::INPUT_LOCK);
			if (inputLocked)
			{
				guiWindowFlag = Core::AddSQValueEnum((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_NAV_INPUT);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_DISABLED, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_NO_NAV, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_NO_NAV_DEFAULT_FOCUS, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_READ_ONLY, true);
				JGui::PushItemFlag(J_GUI_ITEM_FLAG_INPUTABLE, false);
				JGui::SetAllColorToSoft(JVector4F(0.2f, 0.2f, 0.2f, 0));
			}

			if (state.nextFocusReqFrame > 0)
			{
				if (inputLocked)
					state.nextFocusReqFrame = 0;
				else
				{
					--state.nextFocusReqFrame;
					if (state.nextFocusReqFrame == 0)
						JGui::SetNextWindowFocus();
				}
			}

			if (state.isMaximize)
				guiWindowFlag = Core::AddSQValueEnum((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_MOVE, J_GUI_WINDOW_FLAG_NO_RESIZE);

			if (CanUseDock())
			{
				J_GUI_DOCK_NODE_FLAG_ flag = J_GUI_DOCK_NODE_FLAG_NO_WINDOW_MENU_BUTTON;
				if (dockUpdateHelper->IsLockSplitAcitvated())
				{
					flag = Core::AddSQValueEnum((J_GUI_DOCK_NODE_FLAG)flag,
						J_GUI_DOCK_NODE_FLAG_NO_SPLIT_ME,
						J_GUI_DOCK_NODE_FLAG_NO_SPLIT_OTHER);
				}
				if (dockUpdateHelper->IsLockOverAcitvated())
				{
					flag = Core::AddSQValueEnum((J_GUI_DOCK_NODE_FLAG)flag, J_GUI_DOCK_NODE_FLAG_NO_OVER_ME,
						J_GUI_DOCK_NODE_FLAG_NO_OVER_OTHER,
						J_GUI_DOCK_NODE_FLAG_NO_OVER_EMPTY);
				}
				JGui::OverrideNextDockNodeFlag(flag);
				if (dockUpdateHelper->IsLockMove())
					guiWindowFlag = Core::AddSQValueEnum((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_MOVE);

				if (dockUpdateHelper->IsLastDock() || dockUpdateHelper->IsLastWindow())
					JGui::BeginWindow(GetName(), nullptr, guiWindowFlag);
				else
					JGui::BeginWindow(GetName(), &state.isWindowOpen, guiWindowFlag);
			}
			else
			{
				guiWindowFlag = Core::AddSQValue(guiWindowFlag, J_GUI_WINDOW_FLAG_NO_DOCKING);
				if(!state.isMaximize)
					guiWindowFlag = Core::MinusSQValue((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_COLLAPSE);
				else
					guiWindowFlag = Core::AddSQValue((J_GUI_WINDOW_FLAG)guiWindowFlag, J_GUI_WINDOW_FLAG_NO_COLLAPSE);
				if (state.isMaximize)
				{
					//개별 윈도우는 뷰포트 크기만큼 확대
					//dockNode를 지원하는 윈도우는 dockSpace(page)만큼 자동으로 확대된다
					JGui::SetNextWindowPos(JGui::GetMainWorkPos());
					JGui::SetNextWindowSize(JGui::GetMainWorkSize());
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
				JGui::BeginWindow(GetName(), &state.isWindowOpen, guiWindowFlag);
			}

			if (CanMaximize())
			{
				if (state.isMaximize)
				{
					if (JGui::PreviousSizeButton(true, false))
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PREVIOUS_SIZE_WINDOW,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPreviousSizeWindowEvStruct>(this, true)));
					}
				}
				else
				{
					if (JGui::MaximizeButton())
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::MAXIMIZE_WINDOW,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorMaximizeWindowEvStruct>(this, JGui::GetWindowPos(), JGui::GetWindowSize())));
					}
				}
			}

			if (!state.isWindowOpen)
			{
				bool canCloseWindow = dockUpdateHelper != nullptr ? (!dockUpdateHelper->IsLastDock() && !dockUpdateHelper->IsLastWindow()) : true;
				if (canCloseWindow)
				{
					if (state.isMaximize)
					{
						AddEventNotification(*JEditorEvent::EvInterface(),
							GetGuid(),
							J_EDITOR_EVENT::PREVIOUS_SIZE_WINDOW,
							JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPreviousSizeWindowEvStruct>(this, false)));
					}
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::CLOSE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorCloseWindowEvStruct>(GetName(), GetOwnerPageType())));
				}
			}
			SetContentsClick(false);
		}
		void JEditorWindow::CloseWindow()
		{
			const bool isMouseClick = JGui::AnyMouseClicked(false);
			if (CanUseSelectedMap() && isMouseClick && selectedObjMap.size() > 0)
			{
				const bool isMouseInWindow = JGui::IsMouseInRect(JGui::GetWindowPos(), JGui::GetWindowSize());
				const bool selectedHold = JGui::IsKeyDown(Core::J_KEYCODE::CONTROL);
				//!canStaySelected일 경우 화면을 벗어나서 클릭하면 selected map clear
				if (!isMouseInWindow && !option.canStaySelected)
					ExecuteEv(WINDOW_INNER_EVENT::CLEAR_SELECTED_OBJECT);
				//hold를 누르지않고 유효하지않은 객체를 클릭시  selected map clear
				//유효 객체를 클릭시 하위 윈도우 클래스에서 clear & push를 할것이라고 예상한다
				if (isMouseInWindow && !state.isContentsClick && !selectedHold)
					ExecuteEv(WINDOW_INNER_EVENT::CLEAR_SELECTED_OBJECT);
			}
			JGui::EndWindow();
			SetLastActivated(IsActivated());
			J_EDITOR_PAGE_FLAG pageFlag = JEditorPageShareData::GetPageFlag(ownerPageType);
			if (JEditorPageShareData::GetPublicState(ownerPageType, J_EDITOR_PAGE_PUBLIC_STATE::INPUT_LOCK))
			{
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::PopItemFlag();
				JGui::SetAllColorToSoft(JVector4<float>(-0.2f, -0.2f, -0.2f, 0));
			}
		}
		void JEditorWindow::UpdateMouseClick()
		{
			const bool isMouseInWindow = JGui::IsMouseInRect(JGui::GetWindowPos(), JGui::GetWindowSize());
			if (IsFocus() && IsActivated() && isMouseInWindow)
			{
				if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::LEFT))
				{
					std::unique_ptr<JEditorMouseClickEvStruct> lclickEvStruct = std::make_unique<JEditorMouseClickEvStruct>(GetName(), 0, ownerPageType);
					NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, lclickEvStruct.get());
				}
				else if (JGui::IsMouseClicked(Core::J_MOUSE_BUTTON::RIGHT))
				{
					std::unique_ptr<JEditorMouseClickEvStruct> rclickEvStruct = std::make_unique< JEditorMouseClickEvStruct>(GetName(), 1, ownerPageType);
					NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, rclickEvStruct.get());
				}
			}

			if (JGui::IsCurrentWindowFocused(J_GUI_FOCUS_FLAG_CHILD_WINDOW))
			{
				if (!IsFocus())
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::FOCUS_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorFocusWindowEvStruct>(this)));
				}
			}
			else if (!isMouseInWindow && IsFocus())
			{
				AddEventNotification(*JEditorEvent::EvInterface(),
					GetGuid(),
					J_EDITOR_EVENT::UNFOCUS_WINDOW,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorUnFocusWindowEvStruct>(this)));
			}
		}
		void JEditorWindow::UpdateMouseWheel() {}
		void JEditorWindow::UpdateDocking()
		{
			if (!CanUseDock())
				return;

			if (dockUpdateHelper != nullptr)
			{
				JDockUpdateHelper::UpdateData updata;
				dockUpdateHelper->Update(updata);
				if (updata.rollbackBind != nullptr)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::BIND_FUNC,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(updata.rollbackBind), GetOwnerPageType())));
				}
			}

			JGuiWindowInfo wndInfo;
			JGuiDockNodeInfo dockInfo;
			if (JGui::GetCurrentWindowInfo(wndInfo) && JGui::GetCurrentDockNodeInfo(dockInfo))
			{
				GuiID windowID = wndInfo.windowID;
				GuiID selectedTabID = dockInfo.selectedTabID;
				if (selectedTabID == 0)
					return;

				if (IsActivated() && windowID != selectedTabID)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::DEACTIVATE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeActWindowEvStruct>(this)));
				}
				else if (!IsActivated() && windowID == selectedTabID)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::ACTIVATE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this)));
				}
			}
		}
		void JEditorWindow::UpdatePopup(const PopupSetting setting)
		{
			PopupResult temp;
			UpdatePopup(setting, temp);
		}
		void JEditorWindow::UpdatePopup(const PopupSetting setting, _Out_ PopupResult& result)
		{
			if (CanUsePopup() && setting.IsValid() && setting.canOpenPopup)
			{
				if (setting.popupMenu->IsOpen())
				{
					result.isOpen = true;
					setting.popupMenu->ExecutePopup(setting.stringMap);
				}
				result.isLeafPopupContentsClicked = setting.popupMenu->IsLeafPopupContentsClicked();
				setting.popupMenu->Update();
				if (setting.popupMenu->IsPopupContentsClicked())
				{
					result.isPopupContentsClicked = setting.popupMenu->IsPopupContentsClicked();
					SetContentsClick(true);
				}
				result.isMouseInPopup = setting.popupMenu->IsMouseInPopup();
				result.isCloseThisFrame = result.isOpen && !setting.popupMenu->IsOpen();
				if (setting.focusWindowIfCloseThisFrame && result.isCloseThisFrame && JGui::IsMouseInCurrentWindow())
					JGui::FocusCurrentWindow();
			}
		}
		JEditorWindow::PassSelectedOneF::Functor* JEditorWindow::GetPassSelectedOneFunctor()noexcept
		{
			static JEditorWindow::PassSelectedOneF::Functor* passSelectedOneFunctor = nullptr;
			if (passSelectedOneFunctor == nullptr)
			{
				auto passSelectedOneLam = [](JEditorWindow* wnd)
				{
					return wnd->selectedObjMap.size() == 1;
				};
				static PassSelectedOneF::Functor _passSelectedOneFunctor{ passSelectedOneLam };
				passSelectedOneFunctor = &_passSelectedOneFunctor;
			}
			return passSelectedOneFunctor;
		}
		JEditorWindow::PassSelectedAboveOneF::Functor* JEditorWindow::GetPassSelectedAboveOneFunctor()noexcept
		{
			static JEditorWindow::PassSelectedAboveOneF::Functor* passSelectedAboveOneFunctor = nullptr;
			if (passSelectedAboveOneFunctor == nullptr)
			{
				auto _passSelectedAboveOneLam = [](JEditorWindow* wnd)
				{
					return wnd->selectedObjMap.size() >= 1;
				};
				static PassSelectedAboveOneF::Functor _passSelectedAboveOneFunctor{ _passSelectedAboveOneLam };
				passSelectedAboveOneFunctor = &_passSelectedAboveOneFunctor;
			}
			return passSelectedAboveOneFunctor;
		}
		JUserPtr<Core::JIdentifier> JEditorWindow::GetHoveredObject()const noexcept
		{
			return state.hoveredObj;
		}
		uint JEditorWindow::GetSelectedObjectCount()const noexcept
		{
			return (uint)selectedObjMap.size();
		}
		std::vector<JUserPtr<Core::JIdentifier>> JEditorWindow::GetSelectedObjectVec()const noexcept
		{
			std::vector<JUserPtr<Core::JIdentifier>> vec;
			vec.reserve(selectedObjMap.size());

			for (const auto& data : selectedObjMap)
			{
				if (data.second.IsValid())
					vec.push_back(data.second);
			}
			return vec;
		}
		void JEditorWindow::SetNextWindowPos(const JVector2F& pos)noexcept
		{
			if (CanUseDock())
				return;

			state.hasSetNextPosReq = true;
			state.nextPos = pos;
		}
		void JEditorWindow::SetNextWindowSize(const JVector2F& size)noexcept
		{
			if (CanUseDock())
				return;

			state.hasSetNextSizeReq = true;
			state.nextSize = size;
		}
		void JEditorWindow::SetMaximize(const bool value)noexcept
		{
			if (CanMaximize())
				state.isMaximize = value;
		}
		void JEditorWindow::SetOption(const Option& newOption)noexcept
		{
			option = newOption;
		}
		void JEditorWindow::SetHoveredObject(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			state.hoveredObj = obj;
		}
		void JEditorWindow::SetSelectedGameObjectTrigger(const JUserPtr<JGameObject>& gObj, const bool triggerValue)noexcept
		{
			const uint childrenCount = gObj->GetChildrenCount();
			for (uint i = 0; i < childrenCount; ++i)
				SetSelectedGameObjectTrigger(gObj->GetChild(i), triggerValue);

			if (triggerValue)
				JGameObjectPrivate::SelectInterface::Select(gObj);
			else
				JGameObjectPrivate::SelectInterface::DeSelect(gObj);
		}
		void JEditorWindow::SetContentsClick(const bool value)noexcept
		{
			state.isContentsClick = value;
		}
		bool JEditorWindow::IsSelectedObject(const size_t guid)const noexcept
		{
			return selectedObjMap.find(guid) != selectedObjMap.end();
		}
		bool JEditorWindow::CanUseDock()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPROT_DOCK);
		}
		bool JEditorWindow::CanUseSelectedMap()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPPORT_SELECT);
		}
		bool JEditorWindow::CanUsePopup()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPPORT_POPUP) &&
				!JEditorPageShareData::GetPublicState(ownerPageType, J_EDITOR_PAGE_PUBLIC_STATE::INPUT_LOCK);
		}
		bool JEditorWindow::CanMaximize()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPPORT_MAXIMIZE);
		}
		bool JEditorWindow::IsContentsClicked()const noexcept
		{
			return state.isContentsClick;
		}
		void JEditorWindow::PushSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			if (!obj.IsValid() || !CanUseSelectedMap() || selectedObjMap.find(obj->GetGuid()) != selectedObjMap.end())
				return;

			selectedObjMap.emplace(obj->GetGuid(), obj);
		}
		void JEditorWindow::PopSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			if (!obj.IsValid() || !CanUseSelectedMap() || selectedObjMap.find(obj->GetGuid()) == selectedObjMap.end())
				return;

			selectedObjMap.erase(obj->GetGuid());
		}
		void JEditorWindow::ClearSelectedObject()
		{
			if (!CanUseSelectedMap())
				return;

			selectedObjMap.clear();
			/*
				AddEventNotification(*JEditorEvent::EvInterface(),
				GetGuid(),
				J_EDITOR_EVENT::CLEAR_SELECT_OBJECT,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClearSelectObjectEvStruct>(GetOwnerPageType())));
			*/
		}
		void JEditorWindow::PushOtherWindowGuidForListenEv(const size_t guid)noexcept
		{
			if (!CanUseSelectedMap())
				return;

			listenOtherWindowGuidSet.emplace(guid);
		}
		void JEditorWindow::PopOtherWindowGuidForListenEv(const size_t guid)noexcept
		{
			if (!CanUseSelectedMap())
				return;

			listenOtherWindowGuidSet.erase(guid);
		}
		bool JEditorWindow::RegisterEventListener(const J_EDITOR_EVENT evType)
		{
			return AddEventListener(*JEditorEvent::EvInterface(), GetGuid(), evType);
		}
		bool JEditorWindow::RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType)
		{
			return AddEventListener(*JEditorEvent::EvInterface(), GetGuid(), evType);
		}
		void JEditorWindow::DeRegisterEventListener(const J_EDITOR_EVENT evType)
		{
			RemoveEventListener(*JEditorEvent::EvInterface(), GetGuid(), evType);
		}
		void JEditorWindow::DeRegisterListener()
		{
			RemoveListener(*JEditorEvent::EvInterface(), GetGuid());
		}
		void JEditorWindow::RequestPushSelectObject(const JUserPtr<Core::JIdentifier>& selectObj)
		{
			RequestPushSelectObject(std::vector<JUserPtr<Core::JIdentifier>>{selectObj});
		}
		void JEditorWindow::RequestPushSelectObject(const std::vector<JUserPtr<Core::JIdentifier>>& selectObjVec)
		{
			if (selectObjVec.size() == 0)
				return;

			const J_EDITOR_PAGE_TYPE pageType = GetOwnerPageType();
			std::vector<JUserPtr<Core::JIdentifier>> newSelectObjVec;

			const uint newCount = (uint)selectObjVec.size();
			for (uint i = 0; i < newCount; ++i)
			{
				if (selectObjVec[i].IsValid())
					newSelectObjVec.push_back(selectObjVec[i]);
			}
			if (newSelectObjVec.size() == 0)
				return;

			//const bool canSelectMulti = CanUseSelectedMap() && ImGui::GetIO().KeyCtrl;
			const bool canSelectMulti = JGui::IsKeyDown(Core::J_KEYCODE::CONTROL);
			JEditorPushSelectObjectEvStruct* newPushSelectEvStruct = nullptr;
			JEditorEvStruct* popSelectEvStruct = nullptr;
			JEditorEvStruct* clearEvStruct = nullptr;
			JEditorEvStruct* popOverlapSelectedEvStruct = nullptr;
			std::vector<size_t> evGuidVec;
			if (canSelectMulti)
			{
				evGuidVec.resize(3);
				newPushSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(pageType, GetWindowType(), newSelectObjVec, JEditorEvStruct::RANGE::ALL), evGuidVec[0]);
				popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, newSelectObjVec, JEditorEvStruct::RANGE::ALL), evGuidVec[1]);
				std::vector<JUserPtr<Core::JIdentifier>> overlapped;
				for (auto& data : newSelectObjVec)
				{
					if (IsSelectedObject(data->GetGuid()))
						overlapped.emplace_back(data);
				}
				if (overlapped.size() > 0)
					popOverlapSelectedEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, overlapped, JEditorEvStruct::RANGE::ALL), evGuidVec[2]);
			}
			else
			{
				evGuidVec.resize(3);
				newPushSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(pageType, GetWindowType(), newSelectObjVec[0], JEditorEvStruct::RANGE::ALL), evGuidVec[0]);
				popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, newSelectObjVec[0], JEditorEvStruct::RANGE::ALL), evGuidVec[1]);
				clearEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClearSelectObjectEvStruct>(pageType, JEditorEvStruct::RANGE::ALL), evGuidVec[2]);
			}

			std::wstring objName = L"names: ";
			const uint selectedCount = (uint)newPushSelectEvStruct->selectObjVec.size();
			if (selectedCount == 1)
				objName += newPushSelectEvStruct->selectObjVec[0]->GetName();
			else
			{
				for (auto& data : newPushSelectEvStruct->selectObjVec)
					objName += data->GetName() + L'\n';
			}

			auto doBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::PUSH_SELECT_OBJECT, *newPushSelectEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::POP_SELECT_OBJECT, *popSelectEvStruct);
			using ProccessVec = std::vector<std::unique_ptr<Core::JBindHandleBase>>;
			auto task = std::make_unique<Core::JTransitionSetValueTask>("Select ", JCUtil::WstrToU8Str(objName), std::move(doBinder), std::move(undoBinder));
			task->RegisterClearTask(std::make_unique< ClearTaskF::CompletelyBind>(*GetClearTaskFunctor(), std::move(evGuidVec)));
			if (canSelectMulti)
			{
				if (popOverlapSelectedEvStruct != nullptr)
				{
					auto postDoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::POP_SELECT_OBJECT, *popOverlapSelectedEvStruct);
					using ADDITONAL_PROCESS_TYPE = Core::JTransitionTask::ADDITONAL_PROCESS_TYPE;
					using ProcessBindVec = Core::JTransitionTask::ProcessBindVec;
					ProcessBindVec processBindVec;
					processBindVec.push_back(std::move(postDoBinder));
					task->RegisterAddtionalProcess(ADDITONAL_PROCESS_TYPE::DO_POST, std::move(processBindVec));
				}
			}
			else
			{
				auto preDoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::CLEAR_SELECT_OBJECT, *clearEvStruct);
				using ADDITONAL_PROCESS_TYPE = Core::JTransitionTask::ADDITONAL_PROCESS_TYPE;
				using ProcessBindVec = Core::JTransitionTask::ProcessBindVec;
				ProcessBindVec processBindVec;
				processBindVec.push_back(std::move(preDoBinder));
				task->RegisterAddtionalProcess(ADDITONAL_PROCESS_TYPE::DO_PRE, std::move(processBindVec));
			}
			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditorWindow::RequestPopSelectObject(const JUserPtr<Core::JIdentifier>& selectObj)
		{
			RequestPopSelectObject(std::vector<JUserPtr<Core::JIdentifier>>{selectObj});
		}
		void JEditorWindow::RequestPopSelectObject(const std::vector<JUserPtr<Core::JIdentifier>>& selectObjVec)
		{
			JEditorPopSelectObjectEvStruct evStruct(GetOwnerPageType(), selectObjVec, JEditorEvStruct::RANGE::ALL);
			if (!evStruct.PassDefectInspection())
				return;

			std::vector<JUserPtr<Core::JIdentifier>> preSelectObjVec;
			for (const auto& data : selectedObjMap)
				preSelectObjVec.push_back(data.second);

			if (preSelectObjVec.size() == 0)
				return;

			std::vector<size_t> evGuidVec(2);
			JEditorEvStruct* popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(evStruct), evGuidVec[0]);
			JEditorEvStruct* pushSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(evStruct.pageType, GetWindowType(), evStruct.selectObjVec, JEditorEvStruct::RANGE::ALL), evGuidVec[1]);

			std::wstring objName = L"names: ";
			const uint selectedCount = (uint)evStruct.selectObjVec.size();
			if (selectedCount == 1)
				objName += evStruct.selectObjVec[0]->GetName();
			else
			{
				for (auto& data : evStruct.selectObjVec)
					objName += data->GetName() + L'\n';
			}

			auto doBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::POP_SELECT_OBJECT, *popSelectEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::PUSH_SELECT_OBJECT, *pushSelectEvStruct);
			auto task = std::make_unique<Core::JTransitionSetValueTask>("DeSelect", JCUtil::WstrToU8Str(objName), std::move(doBinder), std::move(undoBinder));
			task->RegisterClearTask(std::make_unique< ClearTaskF::CompletelyBind>(*GetClearTaskFunctor(), std::move(evGuidVec)));
			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditorWindow::RequestBind(const std::string& desc,
			std::unique_ptr<Core::JBindHandleBase>&& doHandle,
			std::unique_ptr<Core::JBindHandleBase>&& undoHandle)
		{
			if (doHandle == nullptr || undoHandle == nullptr)
				return;

			std::vector<size_t> evGuidVec(2);
			JEditorEvStruct* doStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(doHandle), GetOwnerPageType()), evGuidVec[0]);
			JEditorEvStruct* undoStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(undoHandle), GetOwnerPageType()), evGuidVec[1]);

			auto wrappedDoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::BIND_FUNC, *doStruct);
			auto wrappedUndoBinder = std::make_unique<EventF::CompletelyBind>(*GetEvFunctor(), *this, J_EDITOR_EVENT::BIND_FUNC, *undoStruct);
			auto task = std::make_unique<Core::JTransitionSetValueTask>("Bind ", desc, std::move(wrappedDoBinder), std::move(wrappedUndoBinder));
			task->RegisterClearTask(std::make_unique< ClearTaskF::CompletelyBind>(*GetClearTaskFunctor(), std::move(evGuidVec)));
			JEditorTransition::Instance().Execute(std::move(task));
		}
		void JEditorWindow::ExecuteEv(const WINDOW_INNER_EVENT evType)
		{
			switch (evType)
			{
			case JinEngine::Editor::JEditorWindow::WINDOW_INNER_EVENT::CLEAR_SELECTED_OBJECT:
			{
				AddEventNotification(*JEditorEvent::EvInterface(),
					GetGuid(),
					J_EDITOR_EVENT::CLEAR_SELECT_OBJECT,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClearSelectObjectEvStruct>(GetOwnerPageType(), JEditorEvStruct::RANGE::ALL)));
				break;
			}
			default:
				break;
			}
		}
		void JEditorWindow::ExecuteThisWindowNotifiedEv(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)
		{
			//이벤트 호출자 관련 이벤트 처리 
			//호출당 한번만 처리되기에 중복해서는 안되는 기능을 여기서 처리
			//ex) marking selected trigger
			if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && CanUseSelectedMap())
			{
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStructure);
				for (auto& data : evstruct->selectObjVec)
				{
					if (data->GetTypeInfo().IsChildOf<JGameObject>())
						SetSelectedGameObjectTrigger(Core::ConnectChildUserPtr<JGameObject>(data), true);
					if (selectedObjMap.find(data->GetGuid()) == selectedObjMap.end())
						selectedObjMap.emplace(data->GetGuid(), data);
				}
			}
			else if (eventType == J_EDITOR_EVENT::POP_SELECT_OBJECT && CanUseSelectedMap())
			{
				JEditorPopSelectObjectEvStruct* evstruct = static_cast<JEditorPopSelectObjectEvStruct*>(eventStructure);
				for (auto& data : evstruct->selectObjVec)
				{
					if (data->GetTypeInfo().IsChildOf<JGameObject>())
						SetSelectedGameObjectTrigger(Core::ConnectChildUserPtr<JGameObject>(data), false);
					selectedObjMap.erase(data->GetGuid());
				}
			}
			else if (eventType == J_EDITOR_EVENT::CLEAR_SELECT_OBJECT && CanUseSelectedMap())
			{
				for (auto& data : selectedObjMap)
				{
					if (data.second->GetTypeInfo().IsChildOf<JGameObject>())
						SetSelectedGameObjectTrigger(Core::ConnectChildUserPtr<JGameObject>(data.second), false);
				}
				ClearSelectedObject();
			}
		}
		void JEditorWindow::ExecuteOtherWindowNotifiedEv(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)
		{
			const bool isListenOtherWndSelect = Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT);
			if (isListenOtherWndSelect)
			{
				if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && CanUseSelectedMap())
				{
					JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStructure);
					for (auto& data : evstruct->selectObjVec)
					{
						if (selectedObjMap.find(data->GetGuid()) == selectedObjMap.end())
							selectedObjMap.emplace(data->GetGuid(), data);
					}
				}
				else if (eventType == J_EDITOR_EVENT::POP_SELECT_OBJECT && CanUseSelectedMap())
				{
					JEditorPopSelectObjectEvStruct* evstruct = static_cast<JEditorPopSelectObjectEvStruct*>(eventStructure);
					for (auto& data : evstruct->selectObjVec)
						selectedObjMap.erase(data->GetGuid());
				}
				else if (eventType == J_EDITOR_EVENT::CLEAR_SELECT_OBJECT && CanUseSelectedMap())
					ClearSelectedObject();
			}
		}
		void JEditorWindow::TryBeginDragging(const JUserPtr<Core::JIdentifier> selectObj)
		{
			if (JGui::BeginDragDropSource())
			{
				RequestPushSelectObject(selectObj);
				JGui::Text(JCUtil::WstrToU8Str(selectObj->GetName()));
				std::string typeName = std::to_string(JEditorPageShareData::GetPageGuiWindowID(GetOwnerPageType()));
				Core::JTypeInstanceSearchHint* draggingHint = JEditorPageShareData::RegisterDraggingHint(GetOwnerPageType(), selectObj.Get());
				JGui::SetDragDropPayload(typeName, draggingHint);
				JGui::EndDragDropSource();
			}
		}
		JUserPtr<Core::JIdentifier> JEditorWindow::TryGetDraggingTarget()
		{
			if (JGui::BeginDragDropTarget() && !JGui::IsMouseDragging(Core::J_MOUSE_BUTTON::LEFT))
			{
				std::string typeName = std::to_string(JEditorPageShareData::GetPageGuiWindowID(GetOwnerPageType()));
				Core::JTypeInstanceSearchHint* draggingHint = JGui::TryGetTypeHintDragDropPayload(typeName);
				JGui::EndDragDropTarget();
				if (draggingHint == nullptr)
					return nullptr;

				JEditorPageShareData::DeRegisterDraggingHint(GetOwnerPageType());
				return JUserPtr<Core::JIdentifier>::ConvertChild(Core::GetUserPtr(*draggingHint));
			}
			else
				return JUserPtr<Core::JIdentifier>{};
		}
		void JEditorWindow::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
			if (Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_FLAG::J_EDITOR_WINDOW_SUPROT_DOCK))
				dockUpdateHelper = JGui::CreateDockUpdateHelper(ownerPageType);
			state.isWindowOpen = true;
		}
		void JEditorWindow::DoSetClose()noexcept
		{
			dockUpdateHelper.reset();
			state.isWindowOpen = false;

			if (CanUseDock())
			{
				JGuiWindowInfo info;
				JGui::GetWindowInfo(GetName(), info);
				JGui::CloseTabItem(info.windowID);
			}
			JEditor::DoSetClose();
		}
		void JEditorWindow::DoActivate()noexcept
		{
			JEditor::DoActivate();
			if (CanUseSelectedMap())
			{
				std::vector<J_EDITOR_EVENT> enumVec
				{
					J_EDITOR_EVENT::PUSH_SELECT_OBJECT, J_EDITOR_EVENT::POP_SELECT_OBJECT, J_EDITOR_EVENT::CLEAR_SELECT_OBJECT
				};
				RegisterEventListener(enumVec);
			}
		}
		void JEditorWindow::DoDeActivate()noexcept
		{
			DeRegisterListener();
			ClearSelectedObject();
			state.hoveredObj.Clear();
			JEditor::DoDeActivate();
		}
		void JEditorWindow::LoadEditorWindow(JFileIOTool& tool)
		{
			std::wstring name;
			bool isOpen;
			bool activated;
			bool isLastActivated;
			bool isFocus;

			JFileIOHelper::LoadJString(tool, name, "Name:");
			JFileIOHelper::LoadAtomicData(tool, isOpen, "Open:");
			JFileIOHelper::LoadAtomicData(tool, activated, "Activate:");
			JFileIOHelper::LoadAtomicData(tool, isLastActivated, "IsLastActivated:");
			JFileIOHelper::LoadAtomicData(tool, isFocus, "Focus:");

			if (isOpen)
			{
				AddEventNotification(*JEditorEvent::EvInterface(),
					GetGuid(),
					J_EDITOR_EVENT::OPEN_WINDOW,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenWindowEvStruct>(GetName(), ownerPageType)));
			}
			if (activated)
			{
				AddEventNotification(*JEditorEvent::EvInterface(),
					GetGuid(),
					J_EDITOR_EVENT::ACTIVATE_WINDOW,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this)));
			}
			SetLastActivated(isLastActivated);
			if (isFocus)
				state.nextFocusReqFrame = state.nextFocusWattingFrame;
		}
		void JEditorWindow::StoreEditorWindow(JFileIOTool& tool)
		{
			JFileIOHelper::StoreJString(tool, GetName(), "Name:");
			JFileIOHelper::StoreAtomicData(tool, IsOpen(), "Open:");
			JFileIOHelper::StoreAtomicData(tool, IsActivated(), "Activate:");
			JFileIOHelper::StoreAtomicData(tool, IsLastActivated(), "IsLastActivated:");
			JFileIOHelper::StoreAtomicData(tool, IsFocus(), "Focus:");
		}
		void JEditorWindow::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStructure)
		{
			if (eventStructure->CanExecuteCallerEv(senderGuid, GetGuid()))
				ExecuteThisWindowNotifiedEv(senderGuid, eventType, eventStructure);
			if (eventStructure->CanExecuteOtherEv(senderGuid, GetGuid()) && listenOtherWindowGuidSet.find(senderGuid) != listenOtherWindowGuidSet.end())
				ExecuteOtherWindowNotifiedEv(senderGuid, eventType, eventStructure);
		}
	}
}