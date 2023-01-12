#include"JEditorManager.h" 
#include"Event/JEditorEventStruct.h"
#include"Event/JEditorEvent.h"
#include"Menubar/JEditorMenuBar.h"
#include"Page/JEditorWindow.h"  
#include"Page/JEditorPageShareData.h"
#include"Page/ProjectMain/JProjectMainPage.h"
#include"Page/SkeletonaAssetSetting/JEditorSkeletonPage.h"  
#include"Page/ProjectSelector/JProjectSelectorPage.h"
#include"Page/SimpleWindow/JProjectCloseConfirm.h"
#include"GuiLibEx/ImGuiEx/JImGuiImpl.h"
#include"../Utility/JCommonUtility.h"
#include"../Core/File/JFileConstant.h"
#include"../Application/JApplicationVariable.h" 
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Resource/Scene/JSceneManager.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Core/Undo/JTransition.h"
#include"../Window/JWindows.h"
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
			JEditorPageUpdateCondition CreatePageUpdateCondition(const JEditorManagerOption& option)
			{
				JEditorPageUpdateCondition condition;
				if (option.acitvatedCloseConfirm)
					condition.canClickPage = false;
				return condition;
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
			}

			JEditorEvent::Clear();
			JEditorPageShareData::Clear();
			JImGuiImpl::Clear();
			Core::JTransition::Clear();

			editorPageMap.clear();
			opendEditorPage.clear();
			editorPage.clear();
		}
		void JEditorManager::OpenProjectSelector()
		{
			editorPage.push_back(std::make_unique<JProjectSelectorPage>());
			editorPageMap.emplace(editorPage[0]->GetPageType(), editorPage[0].get());

			editorPage[0]->SetInitWindow();
			JEditorOpenPageEvStruct evOpenStruct{ editorPage[0]->GetPageType() };
			JEditorActPageEvStruct evActStruct{ editorPage[0].get()};
			OpenPage(&evOpenStruct);
			ActivatePage(&evActStruct);
		}
		void JEditorManager::OpenProject()
		{
			const std::wstring imguiTxt = Core::JFileConstant::MakeFilePath(JApplicationVariable::GetEnginePath(), L"imgui.ini");
			bool hasMetadata = (_waccess(GetMetadataPath().c_str(), 00) != -1);
			bool hasImguiTxt = (_waccess(imguiTxt.c_str(), 00) != -1);
			if (!(hasMetadata && hasImguiTxt))
				_wremove(imguiTxt.c_str());

			editorPage.push_back(std::make_unique<JProjectMainPage>(hasMetadata, &JApplicationProject::RequestStoreProject, &JApplicationProject::RequestLoadProject));
			editorPage.push_back(std::make_unique<JEditorSkeletonPage>(hasMetadata));
 
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
				JEditorActPageEvStruct evActStruct{editorPage[0].get() };
				OpenPage(&evOpenStruct);
				ActivatePage(&evActStruct);
			}
			 
			std::vector<J_EDITOR_EVENT> eventVector = Core::GetEnumElementVec<J_EDITOR_EVENT>();
			this->AddEventListener(*JEditorEvent::EvInterface(), editorManagerGuid, eventVector);

			Core::JTransition::Initialize();
		} 
		void JEditorManager::Update()
		{ 
			JImGuiImpl::SetAlphabetSize();
			JEditorEvent::ExecuteEvent();
			JImGuiImpl::StartEditorUpdate();
			JImGuiImpl::MouseUpdate();
			 
			if (option.acitvatedCloseConfirm)
			{ 
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushItemFlag(ImGuiItemFlags_ReadOnly, true);
				//JImGuiImpl::SetAllColorToDeep(-0.4f);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(JWindow::Instance().GetClientSize() * 0.15f));
			uint8 pageCount = (uint8)opendEditorPage.size();
			for (uint8 i = 0; i < pageCount; ++i)
				opendEditorPage[i]->UpdatePage(Constants::CreatePageUpdateCondition(option));
			ImGui::PopStyleVar();
			JImGuiImpl::EndEditorUpdate();

			if (option.acitvatedCloseConfirm)
			{
				ImGui::PopItemFlag();	
				ImGui::PopItemFlag(); 
			}
			JImGuiImpl::SetAllColorToDefault  ();
			if (option.acitvatedCloseConfirm)
			{
				bool isClose = false;
				bool isCancel = false;
				projectCloseConfirm->Update(isClose, isCancel);
				if (isClose)
					JApplicationProject::SetEndProjectTrigger();
				if (isCancel)
				{
					auto option = GetOption();
					option.acitvatedCloseConfirm = false;
					SetOption(option);
				}
			}
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
						JEditorPageShareData::SetPageOpenData(evStruct);
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
		}
		void JEditorManager::ActivatePage(JEditorActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->actPage->GetPageType());
			if (page != editorPageMap.end())
			{
				if (page->second->IsOpen())
				{
					if (!page->second->IsOpen())
						page->second->SetOpen();
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
				if (!page->second->IsOpen())
					page->second->SetOpen();
				if (!page->second->IsActivated())
					page->second->Activate();
				page->second->SetFocus();
			}
		}
		void JEditorManager::UnFocusPage(JEditorUnFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->unFocusPage->GetPageType());
			if (page != editorPageMap.end())
				page->second->SetUnFocus();
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
		JEditorManagerOption JEditorManager::GetOption()const noexcept
		{
			return option;
		}
		void JEditorManager::SetOption(const JEditorManagerOption& newOption)noexcept
		{
			option = newOption;
		}
		std::wstring JEditorManager::GetMetadataPath()const noexcept
		{
			return Core::JFileConstant::MakeFilePath(JApplicationVariable::GetProjectEditorResourcePath(), Constants::editorPageDataFileName);
		}
		void JEditorManager::OnEvent(const size_t& senderGuid, const J_EDITOR_EVENT& eventType, JEditorEvStruct* eventStruct)
		{   
			if (senderGuid == editorManagerGuid || !eventStruct->PassDefectInspection())
				return;
 
			switch (eventType)
			{
			case J_EDITOR_EVENT::SELECT_OBJECT:
			{
				JEditorSelectObjectEvStruct* selectEvStruct = static_cast<JEditorSelectObjectEvStruct*>(eventStruct);
				JEditorPageShareData::SetSelectObj(selectEvStruct->pageType, selectEvStruct->selectObj);
				break;
			}
			case J_EDITOR_EVENT::DESELECT_OBJECT:
			{
				JEditorDeSelectObjectEvStruct* deSelectEvStruct = static_cast<JEditorDeSelectObjectEvStruct*>(eventStruct);
				JEditorPageShareData::SetSelectObj(deSelectEvStruct->pageType, Core::JUserPtr<JObject>{});
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
		{
			projectCloseConfirm = std::make_unique<JProjectCloseConfirm>();
		}
		JEditorManager::~JEditorManager()
		{}
	}
}