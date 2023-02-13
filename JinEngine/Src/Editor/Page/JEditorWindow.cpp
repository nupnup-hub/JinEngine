#include"JEditorWindow.h"
#include"JEditorPageShareData.h"
#include"JEditorAttribute.h"  
#include"JEditorWindowDockUpdateHelper.h"
#include"../Event/JEditorEvent.h" 
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../../Core/File/JFileIOHelper.h"
#include"../../Core/Undo/JTransition.h"
#include"../../Utility/JCommonUtility.h"  
#include"../../Object/JObject.h"
#include"../../Window/JWindows.h"
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


		JEditorWindow::EventF::Functor* JEditorWindow::evFunctor; 

		JEditorWindow::JEditorWindow(const std::string name,
			std::unique_ptr<JEditorAttribute> attribute,
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditor(name, std::move(attribute)), ownerPageType(ownerPageType), windowFlag(windowFlag)
		{
			if (evFunctor == nullptr)
			{
				auto evFuncLam = [](JEditorWindow& editorWindow, J_EDITOR_EVENT evType, JEditorEvStruct& evStruct)
				{
					editorWindow.AddEventNotification(*JEditorEvent::EvInterface(),
						editorWindow.GetGuid(),
						evType,
						&evStruct);
				};
				static EventF::Functor eventFunctor{ evFuncLam };
				evFunctor = &eventFunctor;
			}
		}
		JEditorWindow::~JEditorWindow() {}
		J_EDITOR_PAGE_TYPE JEditorWindow::GetOwnerPageType()const noexcept
		{
			return ownerPageType;
		}
		void JEditorWindow::EnterWindow(int guiWindowFlag)
		{
			J_EDITOR_PAGE_FLAG pageFlag = JEditorPageShareData::GetPageFlag(ownerPageType);
			if (Core::HasSQValueEnum(pageFlag, J_EDITOR_PAGE_WINDOW_INPUT_LOCK))
				guiWindowFlag = Core::AddSQValueEnum((ImGuiWindowFlags_)guiWindowFlag, ImGuiWindowFlags_NoInputs);

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
		}
		void JEditorWindow::CloseWindow()
		{
			ImGui::End();
			SetLastActivated(IsActivated());
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
				if (IsFocus())
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
		void JEditorWindow::SetSelectableColor(const JVector4<float>& factor)noexcept
		{
			JImGuiImpl::SetColorToSoft(ImGuiCol_Header, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_HeaderHovered, factor);
			JImGuiImpl::SetColorToSoft(ImGuiCol_HeaderActive, factor);
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
		void JEditorWindow::RequestOpenPage(const JEditorOpenPageEvStruct& evStruct, const bool doAct)
		{
			JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(evStruct));
			JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct.pageType));

			auto doBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
			if (doAct)
			{
				JEditorEvStruct* actEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorActPageEvStruct>(evStruct.pageType));
				std::vector<std::unique_ptr<Core::JBindHandleBase>> postDoVec;
				postDoVec.push_back(std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::ACTIVATE_PAGE, *actEvStruct));

				auto task = std::make_unique<Core::JTransitionSetValueTask>("Open Page", std::move(doBinder), std::move(undoBinder));
				task->RegisterAddtionalProcess(Core::JTransitionTask::ADDITONAL_PROCESS_TYPE::DO_POST, std::move(postDoVec));
				Core::JTransition::Execute(std::move(task));
			}
			else
				Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Open Page", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::RequestClosePage(const JEditorClosePageEvStruct& evStruct)
		{
			const J_EDITOR_PAGE_TYPE pageType = evStruct.pageType;
			JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct));
			if (JEditorPageShareData::HasValidOpenPageData(pageType))
			{
				JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(JEditorPageShareData::GetOpendPageData(pageType)));
				auto doBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
				auto undoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
				Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Close Page", std::move(doBinder), std::move(undoBinder)));
			}
			else
			{
				auto doBinder = EventF::CompletelyBind{ *evFunctor, *this,  J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct };
				Core::JTransition::Log("Close Page");
				doBinder.InvokeCompletelyBind();
			}
		}
		void JEditorWindow::RequestSelectObject(const JEditorSelectObjectEvStruct& evStruct)
		{
			if (!evStruct.PassDefectInspection())
				return;

			auto nowSelected = JEditorPageShareData::GetSelectedObj(evStruct.pageType);
			if (nowSelected.IsValid() && nowSelected->GetGuid() == evStruct.selectObj->GetGuid())
				return;

			JEditorEvStruct* selectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(evStruct));
			JEditorEvStruct* deSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct.pageType, evStruct.selectObj->GetGuid()));

			const std::string objName = JCUtil::WstrToU8Str(evStruct.selectObj->GetName());
			auto doBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::SELECT_OBJECT, *selectEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::DESELECT_OBJECT, *deSelectEvStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(objName + "Select", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct)
		{
			if (!evStruct.PassDefectInspection())
				return;

			JEditorEvStruct* deSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct.pageType, evStruct.selectObj->GetGuid()));
			JEditorEvStruct* selectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(evStruct));

			const std::string objName = JCUtil::WstrToU8Str(evStruct.selectObj->GetName());
			auto doBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::DESELECT_OBJECT, *deSelectEvStruct);
			auto undoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::SELECT_OBJECT, *selectEvStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(objName + "DeSelect", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::RequesBind(const std::string& label,
			std::unique_ptr<Core::JBindHandleBase>&& doHandle,
			std::unique_ptr<Core::JBindHandleBase>&& undoHandle)
		{
			if (doHandle == nullptr || undoHandle == nullptr)
				return;

			JEditorEvStruct* doStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(doHandle), GetOwnerPageType()));
			JEditorEvStruct* undoStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorBindFuncEvStruct>(std::move(undoHandle), GetOwnerPageType()));
		
			auto wrappedDoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::BIND_FUNC, *doStruct);
			auto wrappedUndoBinder = std::make_unique<EventF::CompletelyBind>(*evFunctor, *this, J_EDITOR_EVENT::BIND_FUNC, *undoStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(label + "Bind", std::move(wrappedDoBinder), std::move(wrappedUndoBinder)));
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
	}
}