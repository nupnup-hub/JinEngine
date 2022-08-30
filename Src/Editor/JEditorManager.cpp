#include"JEditorManager.h" 
#include"Event/JEditorEventStruct.h"
#include"Menubar/JEditorMenuBar.h"
#include"Page/JEditorWindow.h"  
#include"Page/ProjectMain/JProjectMainPage.h"
#include"Page/SkeletonaAssetSetting/JEditorSkeletonPage.h"  
#include"GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Utility/JCommonUtility.h"
#include"../Application/JApplicationVariable.h" 
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Window/JWindows.h"
#include<fstream>  
#include<io.h>   

//수정필요
//Dock Split시 이전에 Focus하고있던 Window가 offFocus되지않음

namespace JinEngine
{
	namespace Editor
	{
		void JEditorManager::SetEditorBackend()
		{
			JImGuiImpl::Initialize();
		}
		void JEditorManager::OpenProjectSelector()
		{
			JImGuiImpl::SetTextSize();
			JImGuiImpl::SetFont(2);
		}
		void JEditorManager::OpenProject()
		{
			JImGuiImpl::SetTextSize();
			JImGuiImpl::SetFont(2);

			bool hasMetadata = (_access(GetMetadataPath().c_str(), 00) != -1);

			editorPage.push_back(std::make_unique<JProjectMainPage>(hasMetadata));
			editorPage.push_back(std::make_unique<JEditorSkeletonPage>(hasMetadata));

			const std::string imguiTxt = JApplicationVariable::GetEnginePath() + "imgui.ini";
			bool hasImguiTxt = false;
			if (_access(imguiTxt.c_str(), 00) != -1)
				hasImguiTxt = true;

			for (uint i = 0; i < editorPage.size(); ++i)
			{
				editorPage[i]->Initialize(hasImguiTxt);
				editorPageMap.emplace(editorPage[i]->GetGuid(), editorPage[i].get());
			}
	
			if (hasMetadata)
				LoadPage();
			else
			{
				JEditorOpenPageEvStruct evStruct{editorPage[0]->GetPageType()};
				OpenPage(&evStruct);
			}

			std::vector<J_EDITOR_EVENT> eventVector
			{
				J_EDITOR_EVENT::OPEN_PAGE, J_EDITOR_EVENT::CLOSE_PAGE,
				J_EDITOR_EVENT::OPEN_WINDOW, J_EDITOR_EVENT::CLOSE_WINDOW,
			};
			this->AddEventListener(*JImGuiImpl::EvInterface(), editorManagerGuid, eventVector);
		}
		void JEditorManager::Update()
		{
			JImGuiImpl::StartEditorUpdate();
			uint8 pageCount = (uint8)opendEditorPage.size();
			for (uint8 i = 0; i < pageCount; ++i)
				opendEditorPage[i]->UpdatePage();
			JImGuiImpl::EndEditorUpdate();
		}
		void JEditorManager::Clear()
		{
			uint8 pageCount = (uint8)opendEditorPage.size();
			for (uint8 i = 0; i < pageCount; ++i)
				opendEditorPage[i]->DeActivate();

			editorPageMap.clear();
			opendEditorPage.clear();
			editorPage.clear();
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
			if (page != editorPageMap.end())
			{
				if (!page->second->IsOpen())
				{ 
					if (page->second->IsValidOpenRequest(evStruct->openSelected))
					{
						JImGuiImpl::SetPageOpenData(evStruct);
						page->second->SetOpen();
						opendEditorPage.push_back(page->second);
					}
				}
			}
		}
		void JEditorManager::ClosePage(JEditorClosePageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
			{
				if (page->second->IsOpen())
				{
					page->second->DeActivate();
					page->second->OffFront();
					page->second->OffOpen();

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
		}
		void JEditorManager::FrontPage(JEditorFrontPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->page->GetPageType());
			if (page != editorPageMap.end())
			{
				if (page->second->IsOpen())
					page->second->SetFront();
			}
		}
		void JEditorManager::BackPage(JEditorBackPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->page->GetPageType());
			if (page != editorPageMap.end())
			{
				if (page->second->IsActivated())
					page->second->DeActivate();
				page->second->OffFront();
			}
		}
		void JEditorManager::ActivatePage(JEditorActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->actPage->GetPageType());
			if (page != editorPageMap.end())
			{
				if (page->second->IsOpen())
				{
					if (!page->second->IsFront())
						page->second->SetFront();
					page->second->Activate();
				}
			}
		}
		void JEditorManager::DeActivatePage(JEditorDeActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->deActPage->GetPageType());
			if (page != editorPageMap.end())
				page->second->DeActivate();
		}
		void JEditorManager::FocusPage(JEditorFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->focusPage->GetPageType());
			if (page != editorPageMap.end())
			{
				if (!page->second->IsFront())
					page->second->SetFront();
				if (!page->second->IsActivated())
					page->second->Activate();

				if (page->second->IsOpen())
					page->second->SetFocus();
			}
		}
		void JEditorManager::UnFocusPage(JEditorUnFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->unFocusPage->GetPageType());
			if (page != editorPageMap.end())
				page->second->OffFocus();
		}
		void JEditorManager::OpenWindow(JEditorOpenWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->OpenWindow(evStruct->openWindowName);
		}
		void JEditorManager::CloseWindow(JEditorCloseWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->CloseWindow(evStruct->closeWindowName);
		}
		void JEditorManager::FrontWindow(JEditorFrontWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->FrontWindow(evStruct->frontWindow);
		}
		void JEditorManager::BackWindow(JEditorBackWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->BackWindow(evStruct->backWindow);
		}
		void JEditorManager::AcitvateWindow(JEditorActWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->ActivateWindow(evStruct->actWindow);
		}
		void JEditorManager::DeActivateWindow(JEditorDeActWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->DeActivateWindow(evStruct->deActWindow);
		}
		void JEditorManager::FocusWindow(JEditorFocusWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->FocusWindow(evStruct->focusWindow);
		}
		void JEditorManager::UnFocusWindow(JEditorUnFocusWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
				page->second->UnFocusWindow(evStruct->unFocusWindow);
		}
		std::string JEditorManager::GetMetadataPath()noexcept
		{
			return JApplicationVariable::GetProjectEditorResourcePath() + "\\"+ editorPageDataFileName;
		}
		void JEditorManager::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEventStruct* eventStruct)
		{
			if (senderGuid == editorManagerGuid || !eventStruct->PassDefectInspection())
				return;
 
			switch (eventType)
			{
			case J_EDITOR_EVENT::SELECT_OBJECT:
			{	
				JEditorSelectObjectEvStruct* selectEvStruct = static_cast<JEditorSelectObjectEvStruct*>(eventStruct);
				JImGuiImpl::SetSelectObj(selectEvStruct->pageType, selectEvStruct->selectObj);
				break;
			}
			case J_EDITOR_EVENT::DESELECT_OBJECT:
			{
				JEditorDeSelectObjectEvStruct* deSelectEvStruct = static_cast<JEditorDeSelectObjectEvStruct*>(eventStruct);
				JImGuiImpl::SetSelectObj(deSelectEvStruct->pageType, Core::JUserPtr<JObject>{});
				break;
			}
			case J_EDITOR_EVENT::OPEN_PAGE:	 
				OpenPage(static_cast<JEditorOpenPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::CLOSE_PAGE:
				ClosePage(static_cast<JEditorClosePageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::FRONT_PAGE:
				FrontPage(static_cast<JEditorFrontPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::BACK_PAGE:
				BackPage(static_cast<JEditorBackPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::ACTIVATE_PAGE:
				ActivatePage(static_cast<JEditorActPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::DEACTIVATE_PAGE:
				DeActivatePage(static_cast<JEditorDeActPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::FOCUS_PAGE:
				FocusPage(static_cast<JEditorFocusPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::UNFOCUS_PAGE:
				UnFocusPage(static_cast<JEditorUnFocusPageEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::OPEN_WINDOW:
				OpenWindow(static_cast<JEditorOpenWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::CLOSE_WINDOW:
				CloseWindow(static_cast<JEditorCloseWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::FRONT_WINDOW:
				FrontWindow(static_cast<JEditorFrontWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::BACK_WINDOW:
				BackWindow(static_cast<JEditorBackWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::ACTIVATE_WINDOW:
				AcitvateWindow(static_cast<JEditorActWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::DEACTIVATE_WINDOW:
				DeActivateWindow(static_cast<JEditorDeActWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::FOCUS_WINDOW:
				FocusWindow(static_cast<JEditorFocusWindowEvStruct*>(eventStruct));
				break;
			case J_EDITOR_EVENT::UNFOCUS_WINDOW:
				UnFocusWindow(static_cast<JEditorUnFocusWindowEvStruct*>(eventStruct));
				break;
			default:
				break;
			}
		} 
		JEditorManager::JEditorManager()
			:editorManagerGuid(JCommonUtility::CalculateGuid("JEditorManager"))
		{
			 
		}
		JEditorManager::~JEditorManager() {}
	}
}