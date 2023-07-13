#include"JEditorManager.h" 
#include"Event/JEditorEventStruct.h"
#include"Event/JEditorEvent.h"
#include"Menubar/JEditorMenuBar.h"
#include"Interface/JEditorTransitionInterface.h"
#include"Page/JEditorWindow.h"  
#include"Page/JEditorPageShareData.h"
#include"Page/ProjectMain/JProjectMainPage.h"
#include"Page/SkeletonaAssetSetting/JEditorSkeletonPage.h"  
#include"Page/AnimationControllerSetting/JEditorAniContPage.h" 
#include"Page/ProjectSelector/JProjectSelectorPage.h" 
#include"GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Application/JApplicationEngine.h" 
#include"../Application/JApplicationProject.h" 
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/JResourceManager.h" 
#include"../Window/JWindow.h"
#include<fstream>  
#include<io.h>   
 
//수정필요
//Dock Split시 이전에 Focus하고있던 Window가 offFocus되지않음

namespace JinEngine
{
	namespace Editor
	{
		namespace Constants
		{
			const std::wstring editorPageDataFileName = L"EditorData.txt";
		}
		namespace Private
		{
			static std::wstring GetSrcImGuiSaveDataPath()noexcept 
			{
				return Core::JFileConstant::MakeFilePath(JApplicationEngine::ProjectPath(), L"imgui.ini");
			}
			static std::wstring GetCopiedImGuiSaveDataPath()noexcept
			{
				return Core::JFileConstant::MakeFilePath(JApplicationProject::EditorSettingPath(), L"imgui.ini");;
			}
		}

		void JEditorManager::Initialize()
		{
			JImGuiImpl::Initialize();
			JEditorEvent::Initialize();
		}
		void JEditorManager::Clear()
		{
			uint8 pageCount = (uint8)editorPage.size();
			for (uint8 i = 0; i < pageCount; ++i)
			{
				JEditorClosePageEvStruct evStruct{ editorPage[i]->GetPageType() };
				ClosePage(&evStruct);
				editorPage[i]->Clear();
			}

			JEditorPageShareData::Clear();
			JEditorTransition::Instance().Clear();
			JEditorEvent::Clear();
			JImGuiImpl::Clear();

			editorPageMap.clear();
			opendEditorPage.clear();
			editorPage.clear();
			 
			if(JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				JFileIOHelper::CopyFile(Private::GetSrcImGuiSaveDataPath(), Private::GetCopiedImGuiSaveDataPath());
		}
		void JEditorManager::OpenProjectSelector()
		{
			editorPage.push_back(std::make_unique<JProjectSelectorPage>());
			editorPageMap.emplace(editorPage[0]->GetPageType(), editorPage[0].get());

			editorPage[0]->Initialize();
			editorPage[0]->SetInitWindow();
			JEditorOpenPageEvStruct evOpenStruct{ editorPage[0]->GetPageType() };
			JEditorActPageEvStruct evActStruct{ editorPage[0]->GetPageType()};
			OpenPage(&evOpenStruct);
			ActivatePage(&evActStruct);

			std::vector<J_EDITOR_EVENT> eventVector = Core::GetEnumElementVec<J_EDITOR_EVENT>();
			AddEventListener(*JEditorEvent::EvInterface(), editorManagerGuid, eventVector); 
			JEditorTransition::Instance().Initialize();
		}
		void JEditorManager::OpenProject()
		{ 
			const bool hasMetadata = (_waccess(GetMetadataPath().c_str(), 00) != -1);
			const bool hasImguiTxt = (_waccess(Private::GetCopiedImGuiSaveDataPath().c_str(), 00) != -1);
			   
			_wremove(Private::GetSrcImGuiSaveDataPath().c_str());
			if (hasImguiTxt)
				JFileIOHelper::CopyFile(Private::GetCopiedImGuiSaveDataPath(), Private::GetSrcImGuiSaveDataPath());
	
			editorPage.push_back(std::make_unique<JProjectMainPage>());
			editorPage.push_back(std::make_unique<JEditorSkeletonPage>());
			editorPage.push_back(std::make_unique<JEditorAniContPage>());

			//bool hasImguiTxt = false;
			//if (_waccess(imguiTxt.c_str(), 00) != -1)
			//	hasImguiTxt = true;

			const uint pageCount = (uint)editorPage.size();
			for (uint i = 0; i < pageCount; ++i)
			{
				editorPage[i]->Initialize();
				editorPageMap.emplace(editorPage[i]->GetPageType(), editorPage[i].get());
			}
		
			if (hasMetadata)
				LoadPage();
			else
			{
				for (uint i = 0; i < pageCount; ++i)
					editorPage[i]->SetInitWindow();
				JEditorOpenPageEvStruct evOpenStruct{editorPage[0]->GetPageType()};
				JEditorActPageEvStruct evActStruct{editorPage[0]->GetPageType() };
				OpenPage(&evOpenStruct);
				ActivatePage(&evActStruct);
			}
			 
			//always EditorManager first listener about all event type
			std::vector<J_EDITOR_EVENT> eventVector = Core::GetEnumElementVec<J_EDITOR_EVENT>();
			AddEventListener(*JEditorEvent::EvInterface(), editorManagerGuid, eventVector); 
			JEditorTransition::Instance().Initialize();
		} 
		void JEditorManager::Update()
		{ 
			if (ImGui::GetIO().KeyCtrl)
			{
				if (ImGui::IsKeyPressedMap(ImGuiKey_Z, false))
					JEditorTransition::Instance().Undo();
				else if (ImGui::IsKeyPressedMap(ImGuiKey_Y, false))
					JEditorTransition::Instance().Redo();
			}
			JEditorEvent::ExecuteEvent();
			JImGuiImpl::SetAlphabetSize();
			JImGuiImpl::StartEditorUpdate();
			JImGuiImpl::MouseUpdate();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(JWindow::GetClientSize() * 0.15f));
			uint8 pageCount = (uint8)opendEditorPage.size();
			for (uint8 i = 0; i < pageCount; ++i)
				opendEditorPage[i]->UpdatePage();
			ImGui::PopStyleVar();
			JImGuiImpl::EndEditorUpdate();
			JImGuiImpl::SetAllColorToDefault();
		}
		void JEditorManager::LoadPage()
		{
			std::wstring guide;
			std::wifstream stream;
			stream.open(GetMetadataPath(), std::ios::in);
			stream >> guide;
			for (int i = 0; i < editorPage.size(); ++i)
				editorPage[i]->LoadPage(stream);
			stream.close();
		}
		void JEditorManager::StorePage()
		{ 
			std::wofstream stream;
			stream.open(GetMetadataPath(), std::ios::out);
			stream << L"EditorPage: " << '\n';
			for (int i = 0; i < editorPage.size(); ++i)
				editorPage[i]->StorePage(stream);
			stream.close();
		}
		void JEditorManager::OpenPage(JEditorOpenPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			if (!page->second->IsOpen())
			{
				if (page->second->IsValidOpenRequest(evStruct->GetOpenSeleted()))
				{
					JEditorPageShareData::SetPageOpenData(evStruct);
					page->second->SetOpen();
					opendEditorPage.push_back(page->second);
				}
			}
		}
		void JEditorManager::ClosePage(JEditorClosePageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			if (page->second->IsOpen())
			{
				page->second->DeActivate();
				page->second->SetClose();

				const size_t tarGuid = page->second->GetGuid();
				const uint openEditorPageCount = (uint)opendEditorPage.size();
				for (uint j = 0; j < openEditorPageCount; ++j)
				{
					if (opendEditorPage[j]->GetGuid() == tarGuid)
					{
						opendEditorPage.erase(opendEditorPage.begin() + j);
						break;
					}
				}
			}
		}
		void JEditorManager::ActivatePage(JEditorActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			if (page->second->IsOpen())
			{
				if (!page->second->IsOpen())
					page->second->SetOpen();
				page->second->Activate();
			}
		}
		void JEditorManager::DeActivatePage(JEditorDeActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->DeActivate();
		}
		void JEditorManager::FocusPage(JEditorFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->focusPage->GetPageType());
			if (page == editorPageMap.end())
				return;
			
			if (!page->second->IsOpen())
				page->second->SetOpen();
			if (!page->second->IsActivated())
				page->second->Activate();
			page->second->SetFocus();

			//main page allways index 0
			int index = JCUtil::GetTypeIndex(opendEditorPage, page->second->GetGuid(), &JEditorPage::GetGuid);
			if (index != 0 && opendEditorPage.size() - 1 != index)
			{
				opendEditorPage.erase(opendEditorPage.begin() + index);
				opendEditorPage.push_back(page->second);
				opendEditorPage.shrink_to_fit();
			}
		}
		void JEditorManager::UnFocusPage(JEditorUnFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->unFocusPage->GetPageType());
			if (page == editorPageMap.end())
				return;

			page->second->SetUnFocus();
		}
		void JEditorManager::OpenWindow(JEditorOpenWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->OpenWindow(evStruct->openWindowName);
		}
		void JEditorManager::CloseWindow(JEditorCloseWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->CloseWindow(evStruct->closeWindowName);
		}
		void JEditorManager::AcitvateWindow(JEditorActWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;
			page->second->ActivateWindow(evStruct->actWindow);
		}
		void JEditorManager::DeActivateWindow(JEditorDeActWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;
			page->second->DeActivateWindow(evStruct->deActWindow);
		}
		void JEditorManager::FocusWindow(JEditorFocusWindowEvStruct* evStruct)
		{ 
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;
			page->second->FocusWindow(evStruct->focusWindow);
		}
		void JEditorManager::UnFocusWindow(JEditorUnFocusWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;
			page->second->UnFocusWindow(evStruct->unFocusWindow);
		}
		void JEditorManager::OpenPopupWindow(JEditorOpenPopupWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			JEditorTransition::Instance().SetLock(true);
			for (auto& data : editorPageMap)
				data.second->SetPageFlag(Core::AddSQValueEnum(data.second->GetPageFlag(), J_EDITOR_PAGE_WINDOW_INPUT_LOCK));
			if (evStruct->popupWindow != nullptr)
				page->second->OpenPopupWindow(evStruct->popupWindow);
			else
				page->second->OpenPopupWindow(evStruct->popupType);
		}
		void JEditorManager::ClosePopupWindow(JEditorClosePopupWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			if (evStruct->popupWindow != nullptr)
				page->second->ClosePopupWindow(evStruct->popupWindow);
			else
				page->second->ClosePopupWindow(evStruct->popupType);
			for (auto& data : editorPageMap)
				data.second->SetPageFlag(Core::MinusSQValueEnum(data.second->GetPageFlag(), J_EDITOR_PAGE_WINDOW_INPUT_LOCK));
			JEditorTransition::Instance().SetLock(false);
		}
		void JEditorManager::MaximizeWindow(JEditorMaximizeWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->MaximizeWindow(evStruct->wnd);
		}
		void JEditorManager::PreviousSizeWindow(JEditorPreviousSizeWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->PreviousSizeWindow(evStruct->wnd);
		}
		std::wstring JEditorManager::GetMetadataPath()const noexcept
		{
			return Core::JFileConstant::MakeFilePath(JApplicationProject::EditorSettingPath(), Constants::editorPageDataFileName);
		}
		void JEditorManager::PressMainWindowCloseButton()noexcept
		{
			auto data = editorPageMap.find(J_EDITOR_PAGE_TYPE::PROJECT_MAIN)->second;
			AddEventNotification(*JEditorEvent::EvInterface(),
				editorManagerGuid,
				J_EDITOR_EVENT::OPEN_POPUP_WINDOW,
				JEditorEvent::RegisterEvStruct(std::make_unique<JEditorOpenPopupWindowEvStruct>(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM, J_EDITOR_PAGE_TYPE::PROJECT_MAIN)));
			//data->second->OpenPopupWindow(J_EDITOR_POPUP_WINDOW_TYPE::CLOSE_CONFIRM);
		}
		void JEditorManager::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{   
			if (!eventStruct->PassDefectInspection())
				return;
 
			if (senderGuid == editorManagerGuid)
			{
				if (eventType == J_EDITOR_EVENT::OPEN_POPUP_WINDOW)
					OpenPopupWindow(static_cast<JEditorOpenPopupWindowEvStruct*>(eventStruct));
				return;
			}

			switch (eventType)
			{
			case J_EDITOR_EVENT::CLEAR_SELECT_OBJECT:
			{
				JEditorClearSelectObjectEvStruct* clearEvStruct = static_cast<JEditorClearSelectObjectEvStruct*>(eventStruct);
				JEditorPageShareData::ClearPageData(clearEvStruct->pageType);
				break;
			}
			case J_EDITOR_EVENT::OPEN_PAGE:
			{
				OpenPage(static_cast<JEditorOpenPageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::CLOSE_PAGE:
			{
				ClosePage(static_cast<JEditorClosePageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::ACTIVATE_PAGE:
			{
				ActivatePage(static_cast<JEditorActPageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::DEACTIVATE_PAGE:
			{
				DeActivatePage(static_cast<JEditorDeActPageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::FOCUS_PAGE:
			{
				FocusPage(static_cast<JEditorFocusPageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::UNFOCUS_PAGE:
			{
				UnFocusPage(static_cast<JEditorUnFocusPageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::OPEN_WINDOW:
			{
				OpenWindow(static_cast<JEditorOpenWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::CLOSE_WINDOW:
			{
				CloseWindow(static_cast<JEditorCloseWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::ACTIVATE_WINDOW:
			{
				AcitvateWindow(static_cast<JEditorActWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::DEACTIVATE_WINDOW:
			{
				DeActivateWindow(static_cast<JEditorDeActWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::FOCUS_WINDOW:
			{
				FocusWindow(static_cast<JEditorFocusWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::UNFOCUS_WINDOW:
			{
				UnFocusWindow(static_cast<JEditorUnFocusWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::OPEN_POPUP_WINDOW:
			{
				OpenPopupWindow(static_cast<JEditorOpenPopupWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::CLOSE_POPUP_WINDOW:
			{
				ClosePopupWindow(static_cast<JEditorClosePopupWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::MAXIMIZE_WINDOW:
			{
				MaximizeWindow(static_cast<JEditorMaximizeWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::PREVIOUS_SIZE_WINDOW:
			{
				PreviousSizeWindow(static_cast<JEditorPreviousSizeWindowEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::BIND_FUNC:
			{
				JEditorBindFuncEvStruct* bindEv = static_cast<JEditorBindFuncEvStruct*>(eventStruct);
				bindEv->Execute();
				break;
			}
			case J_EDITOR_EVENT::T_BIND_FUNC:
			{
				JEditorTBindFuncEvStruct* bindEv = static_cast<JEditorTBindFuncEvStruct*>(eventStruct);
				bindEv->Execute();
				break;
			}
			default:
				break;
			}
		} 
		JEditorManager::JEditorManager()
			:editorManagerGuid(JCUtil::CalculateGuid("JEditorManager"))
		{ }
		JEditorManager::~JEditorManager()
		{}
	}
}