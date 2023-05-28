#include"JEditorWindow.h"
#include"JEditorPageShareData.h"
#include"JEditorAttribute.h"  
#include"JEditorWindowDockUpdateHelper.h"
#include"../Event/JEditorEvent.h" 
#include"../Popup/JEditorPopupMenu.h"
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Transition/JTransition.h"
#include"../../Utility/JCommonUtility.h"   
#include"../../Object/GameObject/JGameObject.h" 
#include"../../Object/GameObject/JGameObjectPrivate.h" 
#include"../../Window/JWindow.h"
#include<fstream>

namespace JinEngine
{
	namespace Editor
	{
		namespace Private
		{
			static void OverrideDockFlag(ImGuiDockNodeFlags flag)
			{
				ImGuiWindowClass window_class;
				window_class.DockNodeFlagsOverrideSet |= flag;
				ImGui::SetNextWindowClass(&window_class);
			}
		}

		JEditorWindow::PopupSetting::PopupSetting(JEditorPopupMenu* popupMenu,
			JEditorStringMap* stringMap,
			bool canOpenPopup)
			:popupMenu(popupMenu), stringMap(stringMap), canOpenPopup(canOpenPopup)
		{

		}
		bool JEditorWindow::PopupSetting::IsValid()const noexcept
		{
			return popupMenu != nullptr && stringMap != nullptr;
		}

		JEditorWindow::JEditorWindow(const std::string name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditor(name, std::move(attribute)), ownerPageType(ownerPageType), windowFlag(windowFlag)
		{}
		JEditorWindow::~JEditorWindow() {}
		J_EDITOR_PAGE_TYPE JEditorWindow::GetOwnerPageType()const noexcept
		{
			return ownerPageType;
		}
		void JEditorWindow::EnterWindow(int guiWindowFlag)
		{
			J_EDITOR_PAGE_FLAG pageFlag = JEditorPageShareData::GetPageFlag(ownerPageType);
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_WINDOW_INPUT_LOCK))
			{ 
				guiWindowFlag = Core::AddSQValueEnum((ImGuiWindowFlags_)guiWindowFlag, ImGuiWindowFlags_NoInputs);
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
				ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
				ImGui::PushItemFlag(ImGuiItemFlags_ReadOnly, true);
				ImGui::PushItemFlag(ImGuiItemFlags_Inputable, false);  
				JImGuiImpl::SetAllColorToSoft(JVector4<float>(0.2f, 0.2f, 0.2f, 0));
			}

			if (Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPROT_DOCK))
			{
				ImGuiDockNodeFlagsPrivate_ flag = ImGuiDockNodeFlags_NoWindowMenuButton;
				if (dockUpdateHelper->IsLockSplitAcitvated())
				{
					flag = Core::AddSQValueEnum(flag, (ImGuiDockNodeFlagsPrivate_)(ImGuiDockNodeFlags_NoDockingSplitMe |
						ImGuiDockNodeFlags_NoDockingSplitOther));
				}
				if (dockUpdateHelper->IsLockOverAcitvated())
				{
					flag = Core::AddSQValueEnum(flag, (ImGuiDockNodeFlagsPrivate_)(ImGuiDockNodeFlags_NoDockingOverMe |
						ImGuiDockNodeFlags_NoDockingOverOther |
						ImGuiDockNodeFlags_NoDockingOverEmpty));
				}
				Private::OverrideDockFlag(flag);
				if (dockUpdateHelper->IsLastDock() || dockUpdateHelper->IsLastWindow())
				{
					guiWindowFlag = Core::AddSQValueEnum((ImGuiWindowFlags_)guiWindowFlag, ImGuiWindowFlags_NoMove);
					ImGui::Begin(GetName().c_str(), 0, guiWindowFlag);
				}
				else
					ImGui::Begin(GetName().c_str(), &isWindowOpen, guiWindowFlag);
			}
			else
			{
				guiWindowFlag = Core::AddSQValue(guiWindowFlag, ImGuiWindowFlags_NoDocking);
				ImGui::Begin(GetName().c_str(), &isWindowOpen, guiWindowFlag);
			}

			if (!isWindowOpen)
			{
				bool canCloseWindow = dockUpdateHelper != nullptr ? (!dockUpdateHelper->IsLastDock() && !dockUpdateHelper->IsLastWindow()) : true;
				if (canCloseWindow)
				{
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
			if (CanUseSelectedMap())
			{
				const bool isFocus = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
				const bool isMouseClick = ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1);
				const bool isMouseInWindow = JImGuiImpl::IsMouseInRect(ImGui::GetWindowPos(), ImGui::GetWindowSize());
				const bool selectedHold = ImGui::GetIO().KeyCtrl;
				if (selectedObjMap.size() > 0 && isMouseClick && !isContentsClick && !selectedHold && isMouseInWindow)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::CLEAR_SELECT_OBJECT,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClearSelectObjectEvStruct>(GetOwnerPageType())));
				}
			}
			ImGui::End();
			SetLastActivated(IsActivated());
			J_EDITOR_PAGE_FLAG pageFlag = JEditorPageShareData::GetPageFlag(ownerPageType);
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_WINDOW_INPUT_LOCK))
			{
				ImGui::PopItemFlag();
				ImGui::PopItemFlag();
				ImGui::PopItemFlag();
				ImGui::PopItemFlag();
				ImGui::PopItemFlag(); 
				JImGuiImpl::SetAllColorToSoft(JVector4<float>(-0.2f, -0.2f, -0.2f, 0));
			}
		}
		void JEditorWindow::UpdateMouseClick()
		{
			std::string windowName = GetName();

			if (IsFocus() && IsActivated() && JImGuiImpl::IsMouseInRect(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize()))
			{
				if (JImGuiImpl::IsLeftMouseClicked())
				{
					JImGuiImpl::SetMouseClick(0, true);
					std::unique_ptr<JEditorMouseClickEvStruct> lclickEvStruct = std::make_unique<JEditorMouseClickEvStruct>(windowName, 0, ownerPageType);
					NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, lclickEvStruct.get());
				}
				else if (JImGuiImpl::IsRightMouseClicked())
				{
					JImGuiImpl::SetMouseClick(1, true);
					std::unique_ptr<JEditorMouseClickEvStruct> rclickEvStruct = std::make_unique< JEditorMouseClickEvStruct>(windowName, 1, ownerPageType);
					NotifyEvent(*JEditorEvent::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, rclickEvStruct.get());
				}
			}
						 
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (!IsFocus())
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::FOCUS_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorFocusWindowEvStruct>(this, ownerPageType)));
				}
			}
			else
			{
				//allow window popup or subWidget click
				const bool isMouseInWindowSubWidget = CanUsePopup() && JImGuiImpl::IsMouseInRect(JImGuiImpl::GetGuiWindowPos(), JImGuiImpl::GetGuiWindowSize());
				if (!isMouseInWindowSubWidget && IsFocus())
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::UNFOCUS_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorUnFocusWindowEvStruct>(this, ownerPageType)));
				}
			}
		}
		void JEditorWindow::UpdateDocking()
		{
			if (!Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPROT_DOCK))
				return;

			if (dockUpdateHelper != nullptr)
			{
				JEditorWindowDockUpdateHelper::UpdateData updata;
				updata.page = GetOwnerPageType();

				dockUpdateHelper->Update(updata);
				if (updata.rollbackBind != nullptr)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::BIND_FUNC,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(updata.rollbackBind), GetOwnerPageType())));
				}
			}

			ImGuiDockNode* dockNode = ImGui::GetWindowDockNode();
			if (dockNode != nullptr)
			{
				ImGuiID windowID = ImGui::GetCurrentWindow()->ID;
				ImGuiID selectedTabId = dockNode->SelectedTabId;

				if (IsActivated() && windowID != selectedTabId)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::DEACTIVATE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeActWindowEvStruct>(this, ownerPageType)));
				}
				else if (!IsActivated() && windowID == selectedTabId)
				{
					AddEventNotification(*JEditorEvent::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::ACTIVATE_WINDOW,
						JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this, ownerPageType)));
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
			}
		}
		bool JEditorWindow::IsSelectedObject(const size_t guid)const noexcept
		{
			return selectedObjMap.find(guid) != selectedObjMap.end();
		}
		bool JEditorWindow::CanUseSelectedMap()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SELECT);
		}
		bool JEditorWindow::CanUsePopup()const noexcept
		{
			return Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPPORT_POPUP) &&
			!Core::HasSQValueEnum(JEditorPageShareData::GetPageFlag(ownerPageType), J_EDITOR_PAGE_WINDOW_INPUT_LOCK);
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
			return hoveredObj;
		}
		uint JEditorWindow::GetSelectedObjectCount()const noexcept
		{
			return (uint)selectedObjMap.size();
		}
		std::vector<JUserPtr<Core::JIdentifier>> JEditorWindow::GetSelectedObjectVec()const noexcept
		{
			std::vector<JUserPtr<Core::JIdentifier>> vec;
			for (const auto& data : selectedObjMap)
			{
				if (data.second.IsValid())
					vec.push_back(data.second);
			}
			return vec;
		}
		JVector4<float> JEditorWindow::GetSelectedColorFactor()const noexcept
		{
			if (IsFocus())
				return JImGuiImpl::GetSelectColorFactor();
			else
				return JImGuiImpl::GetOffFocusSelectedColorFactor();
		}
		void JEditorWindow::SetButtonColor(const JVector4<float>& factor)noexcept
		{
			JImGuiImpl::SetColorToSoft(ImGuiCol_Button, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_ButtonHovered, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_ButtonActive, factor);
		}
		void JEditorWindow::SetTreeNodeColor(const JVector4<float>& factor)noexcept
		{
			JImGuiImpl::SetColorToSoft(ImGuiCol_Header, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_HeaderHovered, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_HeaderActive, factor);
		}
		void JEditorWindow::SetTreeNodeColorToDefault()noexcept
		{
			JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);
		}
		void JEditorWindow::SetHoveredObject(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			hoveredObj = obj;
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
			isContentsClick = value;
		}
		void JEditorWindow::PushSelectedObject(JUserPtr<Core::JIdentifier> obj)noexcept
		{
			if (!obj.IsValid() || !CanUseSelectedMap() || selectedObjMap.find(obj->GetGuid()) != selectedObjMap.end())
				return;

			selectedObjMap.emplace(obj->GetGuid(), obj);
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
				if(selectObjVec[i].IsValid())
					newSelectObjVec.push_back(selectObjVec[i]);
			}
			if (newSelectObjVec.size() == 0)
				return;
			   
			//const bool canSelectMulti = CanUseSelectedMap() && ImGui::GetIO().KeyCtrl;
			const bool canSelectMulti = ImGui::GetIO().KeyCtrl;
			JEditorPushSelectObjectEvStruct* newPushSelectEvStruct = nullptr;
			JEditorEvStruct* popSelectEvStruct = nullptr;
			JEditorEvStruct* clearEvStruct = nullptr;
			JEditorEvStruct* popOverlapSelectedEvStruct = nullptr;
			std::vector<size_t> evGuidVec;
			if (canSelectMulti)
			{
				evGuidVec.resize(3);
				newPushSelectEvStruct = static_cast<JEditorPushSelectObjectEvStruct*>(JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(pageType, GetWindowType(), newSelectObjVec), evGuidVec[0]));
				popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, newSelectObjVec), evGuidVec[1]);				 
				std::vector<JUserPtr<Core::JIdentifier>> overlapped;
				for (auto& data : newSelectObjVec)
				{
					if (IsSelectedObject(data->GetGuid()))
						overlapped.emplace_back(data);
				}
				if(overlapped.size() > 0)
					popOverlapSelectedEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, overlapped), evGuidVec[2]);
			}
			else
			{
				evGuidVec.resize(3);
				newPushSelectEvStruct = static_cast<JEditorPushSelectObjectEvStruct*>(JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(pageType, GetWindowType(), newSelectObjVec[0]), evGuidVec[0]));
				popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(pageType, newSelectObjVec[0]), evGuidVec[1]);
				clearEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClearSelectObjectEvStruct>(pageType), evGuidVec[2]);
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
		void JEditorWindow::RequestPopSelectObject(const JEditorPopSelectObjectEvStruct& evStruct)
		{
			if (!evStruct.PassDefectInspection())
				return;

			std::vector<JUserPtr<Core::JIdentifier>> preSelectObjVec;
			for (const auto& data : selectedObjMap)
				preSelectObjVec.push_back(data.second); 

			if (preSelectObjVec.size() == 0)
				return; 

			std::vector<size_t> evGuidVec(2);
			JEditorEvStruct* popSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPopSelectObjectEvStruct>(evStruct), evGuidVec[0]);
			JEditorEvStruct* pushSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorPushSelectObjectEvStruct>(evStruct.pageType, GetWindowType(), evStruct.selectObjVec), evGuidVec[1]);

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
		void JEditorWindow::RequesBind(const std::string& desc,
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
		void JEditorWindow::TryBeginDragging(const JUserPtr<Core::JIdentifier> selectObj)
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				RequestPushSelectObject(selectObj); 
				JImGuiImpl::Text(JCUtil::WstrToU8Str(selectObj->GetName()));
				std::string typeName = std::to_string(JEditorPageShareData::GetPageGuiWindowID(GetOwnerPageType()));
				Core::JTypeInstanceSearchHint* draggingHint = JEditorPageShareData::RegisterDraggingHint(GetOwnerPageType(), selectObj.Get());
				ImGui::SetDragDropPayload(typeName.c_str(), draggingHint, sizeof(Core::JTypeInstanceSearchHint));
				ImGui::EndDragDropSource();
			}
		}
		JUserPtr<Core::JIdentifier> JEditorWindow::TryGetDraggingTarget()
		{
			if (ImGui::BeginDragDropTarget() && !ImGui::IsMouseDragging(0))
			{
				std::string typeName = std::to_string(JEditorPageShareData::GetPageGuiWindowID(GetOwnerPageType()));
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(typeName.c_str(), ImGuiDragDropFlags_None);
				ImGui::EndDragDropTarget();

				if (payload == nullptr || payload->Data == nullptr)
					return JUserPtr<Core::JIdentifier>{};
				else
				{
					Core::JTypeInstanceSearchHint* draggingHint = static_cast<Core::JTypeInstanceSearchHint*>(payload->Data);
					return JUserPtr<Core::JIdentifier>::ConvertChild(Core::GetUserPtr(*draggingHint));
				}			 
			}
			else
				return JUserPtr<Core::JIdentifier>{};
		}
		void JEditorWindow::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
			if (Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_FLAG::J_EDITOR_WINDOW_SUPROT_DOCK))
				dockUpdateHelper = std::make_unique<JEditorWindowDockUpdateHelper>(ownerPageType);
			isWindowOpen = true;
		}
		void JEditorWindow::DoSetClose()noexcept
		{
			JEditor::DoSetClose();
			dockUpdateHelper.reset();
			isWindowOpen = false;

			if (Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_SUPROT_DOCK))
			{
				ImGuiID id = ImHashStr(GetName().c_str());
				ImGuiWindow* window = nullptr;
				ImGuiContext* cont = ImGui::GetCurrentContext();
				const int wndCount = (int)cont->Windows.size();
				for (int i = 0; i < wndCount; ++i)
				{
					if (cont->Windows[i]->ID == id)
					{
						window = cont->Windows[i];
						break;
					}
				}
				if (window->DockNode != nullptr)
					window->DockNode->WantCloseTabId = window->ID;
			}
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
			JEditor::DoDeActivate();
			DeRegisterListener();
			ClearSelectedObject();
			hoveredObj.Clear();
		}
		void JEditorWindow::StoreEditorWindow(std::wofstream& stream)
		{
			JFileIOHelper::StoreJString(stream, L"Name:", JCUtil::U8StrToWstr(GetName()));
			JFileIOHelper::StoreAtomicData(stream, L"Open:", IsOpen());
			JFileIOHelper::StoreAtomicData(stream, L"Activate:", IsActivated());
			JFileIOHelper::StoreAtomicData(stream, L"IsLastActivated:", IsLastActivated());
			JFileIOHelper::StoreAtomicData(stream, L"Focus:", IsFocus());
		}
		void JEditorWindow::LoadEditorWindow(std::wifstream& stream)
		{
			std::wstring name;
			bool isOpen;
			bool activated;
			bool isLastActivated;
			bool isFocus;

			JFileIOHelper::LoadJString(stream, name);
			JFileIOHelper::LoadAtomicData(stream, isOpen);
			JFileIOHelper::LoadAtomicData(stream, activated);
			JFileIOHelper::LoadAtomicData(stream, isLastActivated);
			JFileIOHelper::LoadAtomicData(stream, isFocus);

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
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this, ownerPageType)));
			}
			SetLastActivated(isLastActivated);
			/*if (isFocus)
			{
				AddEventNotification(*JEditorEvent::EvInterface(),
					GetGuid(),
					J_EDITOR_EVENT::FOCUS_WINDOW,
					JEditorEvent::RegisterEvStruct(std::make_unique<JEditorFocusWindowEvStruct>(this, ownerPageType)));
			}*/
		}
		void JEditorWindow::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{ 
			//이벤트 호출자 관련 이벤트 처리
			if (senderGuid != GetGuid() && Core::HasSQValueEnum(windowFlag, J_EDITOR_WINDOW_LISTEN_OTHER_WINDOW_SELECT))
				return;

			if(eventStruct->pageType != GetOwnerPageType())
				return;

			//호출당 한번만 처리되기에 중복해서는 안되는 기능을 여기서 처리
			//ex) marking selected trigger
			if (eventType == J_EDITOR_EVENT::PUSH_SELECT_OBJECT && CanUseSelectedMap())
			{ 
				JEditorPushSelectObjectEvStruct* evstruct = static_cast<JEditorPushSelectObjectEvStruct*>(eventStruct);
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
				JEditorPopSelectObjectEvStruct* evstruct = static_cast<JEditorPopSelectObjectEvStruct*>(eventStruct);
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
	}
}