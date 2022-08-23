#include"JEditorWindow.h"
#include"JEditorAttribute.h" 
#include"../GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Event/JEditorEventStruct.h" 
#include"../Transition/JEditorTransition.h"
#include"../../Utility/JCommonUtility.h" 
#include<fstream>
 
namespace JinEngine
{
	namespace Editor
	{
		JEditorWindow::EventFunctor* JEditorWindow::evFunctor; 

		void JEditorWindow::EnterWindow()
		{
			std::string windowName = GetName();
			ImGui::Begin(windowName.c_str(), 0,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

			UpdateDocking();
		}
		void JEditorWindow::UpdateWindow()
		{
			std::string windowName = GetName();
			JImGuiImpl::SetMouseClick(0, false);
			JImGuiImpl::SetMouseClick(1, false);

			if (IsFocus() && IsActivated() && ImGui::IsMouseInWindow(ImGui::GetWindowPos(), ImGui::GetWindowSize()))
			{
				if (ImGui::IsMouseClicked(0))
				{
					JImGuiImpl::SetMouseClick(0, true);
					std::unique_ptr<JEditorMouseClickEvStruct> lclickEvStruct = std::make_unique<JEditorMouseClickEvStruct>(windowName, 0, ownerPageType);
					NotifyEvent(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, lclickEvStruct.get());
				}
				else if (ImGui::IsMouseClicked(1))
				{
					JImGuiImpl::SetMouseClick(1, true);
					std::unique_ptr<JEditorMouseClickEvStruct> rclickEvStruct = std::make_unique< JEditorMouseClickEvStruct>(windowName, 1, ownerPageType);
					NotifyEvent(*JImGuiImpl::EvInterface(), GetGuid(), J_EDITOR_EVENT::MOUSE_CLICK, rclickEvStruct.get());
				}
			}
			if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			{
				if (!IsFocus())
				{
					AddEventNotification(*JImGuiImpl::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::FOCUS_WINDOW,
						JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorFocusWindowEvStruct>(this, ownerPageType)));
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
					AddEventNotification(*JImGuiImpl::EvInterface(),
						GetGuid(), 
						J_EDITOR_EVENT::BACK_WINDOW,
						JImGuiImpl::RegisterEvStruct(std::make_unique< JEditorDeActWindowEvStruct>(this, ownerPageType)));
				}
				else if (!IsActivated() && windowID == selectedTabId)
				{
					bool isFocus = false;
					if (dockNode->IsFocused)
						isFocus = true;  
					
					AddEventNotification(*JImGuiImpl::EvInterface(),
						GetGuid(),
						J_EDITOR_EVENT::ACTIVATE_WINDOW,
						JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this, ownerPageType)));
				}
				//forDebug
				/*ImGui::Text(("State: " + std::to_string(dockNode->State)).c_str());
				ImGui::Text(("IsFloatingNode: " + std::to_string(dockNode->IsFloatingNode())).c_str());
				if (dockNode->ParentNode != nullptr)
					ImGui::Text(("Parent: " + std::to_string(dockNode->ParentNode->ID)).c_str());*/
			}
		}
		void JEditorWindow::CloseWindow()
		{
			ImGui::End();
		}
		J_EDITOR_PAGE_TYPE JEditorWindow::GetOwnerPageType()const noexcept
		{
			return ownerPageType;
		}
		void JEditorWindow::SetButtonSelectColor()noexcept
		{
			JImGuiImpl::SetColorToDeep(ImGuiCol_Button, 0.15f);
			JImGuiImpl::SetColorToDeep(ImGuiCol_ButtonHovered, 0.15f);
			JImGuiImpl::SetColorToDeep(ImGuiCol_ButtonActive, 0.15f);
		}
		void JEditorWindow::SetButtonDefaultColor()noexcept
		{
			JImGuiImpl::SetColorToDefault(ImGuiCol_Button);
			JImGuiImpl::SetColorToDefault(ImGuiCol_ButtonHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_ButtonActive);
		}
		void JEditorWindow::SetTreeNodeSelectColor()noexcept
		{
			JImGuiImpl::SetColorToDeep(ImGuiCol_Header, 0.15f);
			JImGuiImpl::SetColorToDeep(ImGuiCol_HeaderHovered, 0.15f);
			JImGuiImpl::SetColorToDeep(ImGuiCol_HeaderActive, 0.15f);
		}
		void JEditorWindow::SetTreeNodeDefaultColor()noexcept
		{
			JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderHovered);
			JImGuiImpl::SetColorToDefault(ImGuiCol_HeaderActive);
		}
		void JEditorWindow::RegisterEventListener(const J_EDITOR_EVENT evType)
		{
			AddEventListener(*JImGuiImpl::EvInterface(), GetGuid(), evType);
		}
		void JEditorWindow::RegisterEventListener(std::vector<J_EDITOR_EVENT>& evType)
		{
			AddEventListener(*JImGuiImpl::EvInterface(), GetGuid(), evType);
		}
		void JEditorWindow::DeRegisterEventListener(const J_EDITOR_EVENT evType)
		{
			RemoveEventListener(*JImGuiImpl::EvInterface(), GetGuid(), evType);
		}
		void JEditorWindow::DeRegisterListener()
		{
			RemoveListener(*JImGuiImpl::EvInterface(), GetGuid());
		}
		void JEditorWindow::RequestOpenPage(const JEditorOpenPageEvStruct& evStruct)
		{
			JEditorEventStruct* openEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(evStruct));
			JEditorEventStruct* closeEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct.pageType));

			auto doBinder = EventFunctorBinder{ *evFunctor, *this, J_EDITOR_EVENT::OPEN_PAGE,  *openEvStruct };
			auto undoBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct };
			JEditorTransition::Execute(JEditorTask{ doBinder, "Open Page" }, JEditorTask{ undoBinder, "Close Page" });
		}
		void JEditorWindow::RequestClosePage(const JEditorClosePageEvStruct& evStruct)
		{
			const J_EDITOR_PAGE_TYPE pageType = evStruct.pageType;
			JEditorEventStruct* closeEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorClosePageEvStruct>(evStruct));
			if (JImGuiImpl::HasValidOpenPageData(pageType))
			{
				JEditorEventStruct* openEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorOpenPageEvStruct>(JImGuiImpl::GetOpendPageData(pageType)));
				auto doBinder = EventFunctorBinder{ *evFunctor, *this, J_EDITOR_EVENT::CLOSE_PAGE,  *closeEvStruct };
				auto undoBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::OPEN_PAGE, *openEvStruct };
				JEditorTransition::Execute(JEditorTask{ doBinder, "Close Page" }, JEditorTask{ undoBinder, "Open Page" });
			}
			else 
			{	auto doBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::CLOSE_PAGE, *closeEvStruct };
				JEditorTransition::Log("Close Page");
				doBinder.InvokeCompletelyBind();
			}
		}
		void JEditorWindow::RequestSelectObject(const JEditorSelectObjectEvStruct& evStruct)
		{			 
			JEditorEventStruct* selectEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(evStruct));
			JEditorEventStruct* deSelectEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct));
		
			auto doBinder = EventFunctorBinder{ *evFunctor, *this, J_EDITOR_EVENT::SELECT_OBJECT,  *selectEvStruct };
			auto undoBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::DESELECT_OBJECT, *deSelectEvStruct };
			JEditorTransition::Execute(JEditorTask{ doBinder, evStruct.objName +" Select" }, JEditorTask{ undoBinder, evStruct.objName + " DeSelect" });
		}
		void JEditorWindow::RequestDeSelectObject(const JEditorSelectObjectEvStruct& evStruct)
		{
			JEditorEventStruct* deSelectEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorDeSelectObjectEvStruct>(evStruct));
			JEditorEventStruct* selectEvStruct = JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorSelectObjectEvStruct>(evStruct));

			auto doBinder = EventFunctorBinder{ *evFunctor, *this, J_EDITOR_EVENT::DESELECT_OBJECT,  *deSelectEvStruct };
			auto undoBinder = EventFunctorBinder{ *evFunctor, *this,  J_EDITOR_EVENT::SELECT_OBJECT, *selectEvStruct };
			JEditorTransition::Execute(JEditorTask{ doBinder, evStruct.objName + " DeSelect" }, JEditorTask{ undoBinder, evStruct.objName + " Select" });
		}
		void JEditorWindow::StoreEditorWindow(std::wofstream& stream)
		{
			stream << L"Open: " << IsOpen() << '\n';
			stream << L"Front: " << IsFront() << '\n';
			stream << L"Activate: " << IsActivated() << '\n';
			stream << L"Focus: " << IsFocus() << '\n';
		}
		void JEditorWindow::LoadEditorWindow(std::wifstream& stream)
		{
			std::wstring guide;
			bool isOpen;
			bool isFront;
			bool activated;
			bool isFocus;

			stream >> guide; stream >> isOpen;
			stream >> guide; stream >> isFront;
			stream >> guide; stream >> activated;
			stream >> guide; stream >> isFocus;

			if (isOpen)
			{  
				AddEventNotification(*JImGuiImpl::EvInterface(), 
					GetGuid(),
					J_EDITOR_EVENT::OPEN_PAGE,
					JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorOpenWindowEvStruct>(GetName(), ownerPageType)));
			}
			if (isFront)
			{
				AddEventNotification(*JImGuiImpl::EvInterface(),
					GetGuid(), 
					J_EDITOR_EVENT::FRONT_PAGE,
					JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorFrontWindowEvStruct>(this, ownerPageType)));
			}
			if (activated)
			{
				AddEventNotification(*JImGuiImpl::EvInterface(),
					GetGuid(), 
					J_EDITOR_EVENT::ACTIVATE_WINDOW, 
					JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorActWindowEvStruct>(this, ownerPageType)));
			}
			if (isFocus)
			{ 
				AddEventNotification(*JImGuiImpl::EvInterface()
					, GetGuid(),
					J_EDITOR_EVENT::FOCUS_WINDOW,
					JImGuiImpl::RegisterEvStruct(std::make_unique<JEditorFocusWindowEvStruct>(this, ownerPageType)));
			}
		}
		void JEditorWindow::RegisterJFunc()
		{
			auto evFuncLam = [](JEditorWindow& editorWindow, J_EDITOR_EVENT evType, JEditorEventStruct& evStruct)
			{
				editorWindow.AddEventNotification(*JImGuiImpl::EvInterface(), 
					editorWindow.GetGuid(), 
					evType,
					&evStruct);
			};
			void(*ptr)(JEditorWindow&, J_EDITOR_EVENT, JEditorEventStruct&) = evFuncLam;

			static EventFunctor eventFunctor{ ptr };
			evFunctor = &eventFunctor;
		}
		JEditorWindow::JEditorWindow(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditor(std::move(attribute)), ownerPageType(ownerPageType)
		{}
		JEditorWindow::~JEditorWindow(){}
	}
}