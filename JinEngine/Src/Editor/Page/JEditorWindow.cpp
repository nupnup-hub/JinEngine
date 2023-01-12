#include"JEditorWindow.h"
#include"JEditorPageShareData.h"
#include"JEditorAttribute.h"  
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
		JEditorWindow::EventFunctor* JEditorWindow::evFunctor;

		JEditorWindow::JEditorWindow(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditor(name, std::move(attribute)), ownerPageType(ownerPageType)
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
				void(*ptr)(JEditorWindow&, J_EDITOR_EVENT, JEditorEvStruct&) = evFuncLam;

				static EventFunctor eventFunctor{ evFuncLam };
				evFunctor = &eventFunctor;
			}
		}
		JEditorWindow::~JEditorWindow() {}
		J_EDITOR_PAGE_TYPE JEditorWindow::GetOwnerPageType()const noexcept
		{
			return ownerPageType;
		}
		void JEditorWindow::EnterWindow(const JEditorWindowUpdateCondition& condition, int windowFlag)
		{ 
			if (!condition.canClickWindow)
				windowFlag = Core::AddSQValueEnum((ImGuiWindowFlags_)windowFlag, ImGuiWindowFlags_NoInputs);
			//ImGuiWindowFlags_NoInputs = ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus,
			ImGui::Begin(GetName().c_str(), 0, windowFlag);
		}
		void JEditorWindow::CloseWindow()
		{
			ImGui::End();
			SetLastActivated(IsActivated());
		}
		void JEditorWindow::UpdateMouseClick()
		{
			std::string windowName = GetName();

			if (IsFocus() && IsActivated() && JImGuiImpl::IsMouseInRect())
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
		void JEditorWindow::SetButtonColor(const float factor)noexcept
		{
			JImGuiImpl::SetColorToDeep(ImGuiCol_Button, factor);
			JImGuiImpl::SetColorToDeep(ImGuiCol_ButtonHovered, factor);
			JImGuiImpl::SetColorToDeep(ImGuiCol_ButtonActive, factor);
		}
		void JEditorWindow::SetTreeNodeColor(const float factor)noexcept
		{
			JImGuiImpl::SetColorToDeep(ImGuiCol_Header, factor);
			JImGuiImpl::SetColorToDeep(ImGuiCol_HeaderHovered, factor);
			JImGuiImpl::SetColorToDeep(ImGuiCol_HeaderActive, factor);
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
		void JEditorWindow::RequestOpenPage(const JEditorOpenPageEvStruct& evStruct)
		{
			JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(evStruct));
			JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct.pageType));

			auto doBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
			auto undoBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Open Page", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::RequestClosePage(const JEditorClosePageEvStruct& evStruct)
		{
			const J_EDITOR_PAGE_TYPE pageType = evStruct.pageType;
			JEditorEvStruct* closeEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct));
			if (JEditorPageShareData::HasValidOpenPageData(pageType))
			{
				JEditorEvStruct* openEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(JEditorPageShareData::GetOpendPageData(pageType)));
				auto doBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct);
				auto undoBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct);
				Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Close Page", std::move(doBinder), std::move(undoBinder)));

			}
			else
			{
				auto doBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct };
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
			JEditorEvStruct* deSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct.pageType));

			const std::string objName = JCUtil::WstrToU8Str(evStruct.selectObj->GetName());
			auto doBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::SELECT_OBJECT, *selectEvStruct);
			auto undoBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::DESELECT_OBJECT, *deSelectEvStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(objName + "Select", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct)
		{
			if (!evStruct.PassDefectInspection())
				return;

			JEditorEvStruct* deSelectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct.pageType));
			JEditorEvStruct* selectEvStruct = JEditorEvent::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(evStruct));

			const std::string objName = JCUtil::WstrToU8Str(evStruct.selectObj->GetName());
			auto doBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::DESELECT_OBJECT, *deSelectEvStruct);
			auto undoBinder = std::make_unique<EventFunctorBinder>(*evFunctor, *this, J_EDITOR_EVENT::SELECT_OBJECT, *selectEvStruct);
			Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(objName + "DeSelect", std::move(doBinder), std::move(undoBinder)));
		}
		void JEditorWindow::DoSetOpen()noexcept
		{
			JEditor::DoSetOpen();
		}
		void JEditorWindow::DoSetClose()noexcept
		{
			JEditor::DoSetClose();
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