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
#include"Gui/JGui.h"
#include"Gui/Adapter/JGuiBehaviorAdapter.h"
#include"../Core/Utility/JCommonUtility.h"
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
		namespace Private
		{
			const std::wstring editorOptionDataFileName = L"EditorOption.txt";
			const std::wstring editorPageDataFileName = L"EditorData.txt";
			static constexpr uint previousSizeExeFrame = 1;
		}

		void JEditorManager::Initialize(std::unique_ptr<JGuiBehaviorAdapter>&& adapter)
		{  
			JGuiPrivate::SetAdapter(std::move(adapter)); 
			JEditorEvent::Initialize();
		}
		void JEditorManager::Clear()
		{
			JGuiPrivate::StoreOption();
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

			if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				JGuiPrivate::StoreGuiData();
			JGuiPrivate::Clear();
			JGuiPrivate::SetAdapter(nullptr);

			editorPageMap.clear();
			opendEditorPage.clear();
			editorPage.clear();
		}
		Graphic::JGuiBackendInterface* JEditorManager::GetBackendInterface()
		{
			return JGuiPrivate::GetBackendInterface();
		}
		std::wstring JEditorManager::GetMetadataPath()const noexcept
		{
			return Core::JFileConstant::MakeFilePath(JApplicationProject::EditoConfigPath(), Private::editorPageDataFileName);
		}
		bool JEditorManager::CanUpdate(JEditorPage* page)const noexcept
		{
			if (page == nullptr)
				return false;

			for (const auto& data : pageSizeTInfo)
			{
				if (data->isMaximize && data->destroyAfFrame == -1 && data->page->GetGuid() != page->GetGuid())
					return false;
			} 
			return true;
		} 
		void JEditorManager::OpenProjectSelector(std::unique_ptr<Graphic::JGuiInitData>&& initData, std::unique_ptr<JEditorProjectInterface>&& pInterface)
		{
			JGuiPrivate::Initialize(std::move(initData));
			JGuiPrivate::LoadOption();

			editorPage.push_back(std::make_unique<JProjectSelectorPage>(std::move(pInterface)));
			editorPageMap.emplace(editorPage[0]->GetPageType(), editorPage[0].get());

			editorPage[0]->Initialize();
			editorPage[0]->SetInitWindow();

			JEditorOpenPageEvStruct evOpenStruct{ editorPage[0]->GetPageType() };
			JEditorActPageEvStruct evActStruct{ editorPage[0]->GetPageType() };
			JEditorFocusPageEvStruct evFocusStruct{ editorPage[0].get() };
			OpenPage(&evOpenStruct);
			ActivatePage(&evActStruct);
			FocusPage(&evFocusStruct); 

			std::vector<J_EDITOR_EVENT> eventVector = Core::GetEnumElementVec<J_EDITOR_EVENT>();
			AddEventListener(*JEditorEvent::EvInterface(), editorManagerGuid, eventVector); 
			JEditorTransition::Instance().Initialize();
		}
		void JEditorManager::OpenProject(std::unique_ptr<Graphic::JGuiInitData>&& initData, std::unique_ptr<JEditorProjectInterface>&& pInterface)
		{
			JGuiPrivate::Initialize(std::move(initData));
			JGuiPrivate::LoadOption();
			JGuiPrivate::LoadGuiData();

			const bool hasMetadata = (_waccess(GetMetadataPath().c_str(), 00) != -1);
			editorPage.push_back(std::make_unique<JProjectMainPage>(std::move(pInterface)));
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
				JEditorFocusPageEvStruct evFocusStruct{ editorPage[0].get() };
				OpenPage(&evOpenStruct);
				ActivatePage(&evActStruct);
				FocusPage(&evFocusStruct);
			}
			 
			//always EditorManager first listener about all event type
			std::vector<J_EDITOR_EVENT> eventVector = Core::GetEnumElementVec<J_EDITOR_EVENT>();
			AddEventListener(*JEditorEvent::EvInterface(), editorManagerGuid, eventVector); 
			JEditorTransition::Instance().Initialize();
		} 
		void JEditorManager::Update()
		{ 
			if (!JWindow::IsActivated())
				return;

			if (JGuiPrivate::StartGuiUpdate())
			{
				JEditorTransition::Instance().Update();
				if (JGui::IsKeyPressed(Core::J_KEYCODE::CONTROL))
				{
					if (JGui::IsKeyDown(Core::J_KEYCODE::Z))
						JEditorTransition::Instance().UndoPerFrame();
					else if (JGui::IsKeyDown(Core::J_KEYCODE::Y))
						JEditorTransition::Instance().RedoPerFrame();
				}
				JEditorEvent::ExecuteEvent(); 
				for (const auto& data : opendEditorPage)
				{
					if (CanUpdate(data))
						data->UpdatePage(); 
				}
				for (const auto& data : opendEditorPage)
				{
					if (CanUpdate(data))
						data->UpdateOpenPopupWindow(JVector2<float>(0, 0), JGui::GetMainWorkSize());
				}
				UpdatePageSizeTransformLife();
				JGuiPrivate::EndGuiUpdate();
				JGui::SetAllColorToDefault();		 
			}
		} 
		void JEditorManager::UpdatePageSizeTransformLife()
		{ 
			for (int i = 0; i < pageSizeTInfo.size(); ++i)
			{
				if (pageSizeTInfo[i]->destroyAfFrame != -1)
				{
					if (pageSizeTInfo[i]->destroyAfFrame != 0)
						--pageSizeTInfo[i]->destroyAfFrame;
					else
					{
						pageSizeTInfo[i]->page->SetFocus();
						pageSizeTInfo.erase(pageSizeTInfo.begin() + i);
						--i;
					}
				}
			}
			 
		}
		void JEditorManager::LoadPage()
		{
			JFileIOTool tool;
			if (!tool.Begin(GetMetadataPath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return;

			for (int i = 0; i < editorPage.size(); ++i)
			{
				tool.PushExistStack();
				editorPage[i]->LoadPage(tool);
				tool.PopStack();
			}
			tool.Close();
		}
		void JEditorManager::StorePage()
		{ 
			JFileIOTool tool;
			if (!tool.Begin(GetMetadataPath(), JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_SET_CONTENTS_ARRAY_OWNER))
				return;

			for (int i = 0; i < editorPage.size(); ++i)
			{
				tool.PushArrayMember();
				editorPage[i]->StorePage(tool);
				tool.PopStack();
			}
			tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
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
				if (page->second->IsActivated())
				{
					JEditorDeActPageEvStruct deActEv(page->second->GetPageType());
					DeActivatePage(&deActEv);
				} 
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
				page->second->Activate();
		}
		void JEditorManager::DeActivatePage(JEditorDeActPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page != editorPageMap.end())
			{
				if (page->second->IsFocus())
				{
					JEditorUnFocusPageEvStruct deFocusEv(page->second);
					UnFocusPage(&deFocusEv);
				}
				page->second->DeActivate();
			}
		}
		void JEditorManager::FocusPage(JEditorFocusPageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->focusPage->GetPageType());
			if (page == editorPageMap.end())
				return;
			
			if (!page->second->IsOpen() || !page->second->IsActivated())
				return;

			page->second->SetFocus();
			focusPage = page->second;

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
			focusPage = nullptr;
		}
		void JEditorManager::MaximizePage(JEditorMaximizePageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->page->GetPageType());
			if (page == editorPageMap.end())
				return;
 
			int existTInfoIndex = FindPageSizeTInfo(evStruct->page->GetGuid());
			if (existTInfoIndex != invalidIndex)
			{
				if (pageSizeTInfo[existTInfoIndex]->isMaximize)
					evStruct->page->SetMaximize(false);
				else
					evStruct->page->SetMinimize(false);
				pageSizeTInfo.erase(pageSizeTInfo.begin() + existTInfoIndex);
			}

			auto newPageSizeTInfo = std::make_unique<PageSizeTransformInfo>();
			newPageSizeTInfo->page = evStruct->page;
			newPageSizeTInfo->page->SetMaximize(true);
			newPageSizeTInfo->prePos = evStruct->prePagePos;
			newPageSizeTInfo->preSize = evStruct->prePageSize;
			newPageSizeTInfo->isMaximize = true;
			pageSizeTInfo.push_back(std::move(newPageSizeTInfo));
		}
		void JEditorManager::MinimizePage(JEditorMinimizePageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->page->GetPageType());
			if (page == editorPageMap.end())
				return;

			int existTInfoIndex = FindPageSizeTInfo(evStruct->page->GetGuid());
			if (existTInfoIndex != invalidIndex)
			{
				if (pageSizeTInfo[existTInfoIndex]->isMaximize)
					evStruct->page->SetMaximize(false);
				else
					evStruct->page->SetMinimize(false);
				pageSizeTInfo.erase(pageSizeTInfo.begin() + existTInfoIndex);
			}

			auto newPageSizeTInfo = std::make_unique<PageSizeTransformInfo>();
			newPageSizeTInfo->page = evStruct->page;
			newPageSizeTInfo->page->SetMinimize(true);
			newPageSizeTInfo->page->SetNextPageSize(JVector2F(evStruct->prePageSize.x, evStruct->height));
			newPageSizeTInfo->prePos = evStruct->prePagePos;
			newPageSizeTInfo->preSize = evStruct->prePageSize;
			newPageSizeTInfo->isMaximize = false;
			pageSizeTInfo.push_back(std::move(newPageSizeTInfo));
		}
		void JEditorManager::PreviousSizePage(JEditorPreviousSizePageEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->page->GetPageType());
			if (page == editorPageMap.end())
				return;

			int index = FindPageSizeTInfo(evStruct->page->GetGuid());
			if (index == invalidIndex)
				return;
		
			pageSizeTInfo[index]->page->SetNextPagePos(pageSizeTInfo[index]->prePos);
			pageSizeTInfo[index]->page->SetNextPageSize(pageSizeTInfo[index]->preSize);
			if(pageSizeTInfo[index]->isMaximize)
				pageSizeTInfo[index]->page->SetMaximize(false);
			else
				pageSizeTInfo[index]->page->SetMinimize(false);

			if (evStruct->useLazy)
				pageSizeTInfo[index]->destroyAfFrame = Private::previousSizeExeFrame;
			else
			{
				pageSizeTInfo[index]->destroyAfFrame = 0;
				UpdatePageSizeTransformLife();
			}
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

			//현재는 popup window가 생성될시
			//우선적으로 popup window 확인, 종료를 강요한다.(페이지와 페이지내부에 윈도우에 제한을 부여한다)
			//추후에 유즈케이스에 변경이 생길시 수정이 필요된다.
			JEditorTransition::Instance().SetLock(true);
			for (auto& data : editorPageMap)
				data.second->SetInputLock(true); 
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
				data.second->SetInputLock(false);
			JEditorTransition::Instance().SetLock(false);
		}
		void JEditorManager::MaximizeWindow(JEditorMaximizeWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->MaximizeWindow(evStruct->wnd, evStruct->preWindowPos, evStruct->preWindowSize);
		}
		void JEditorManager::PreviousSizeWindow(JEditorPreviousSizeWindowEvStruct* evStruct)
		{
			auto page = editorPageMap.find(evStruct->pageType);
			if (page == editorPageMap.end())
				return;

			page->second->PreviousSizeWindow(evStruct->wnd, evStruct->useLazy);
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
		int JEditorManager::FindPageSizeTInfo(const size_t guid)
		{
			bool(*pageEqualPtr)(PageSizeTransformInfo*, size_t) = [](PageSizeTransformInfo* info, size_t guid)
			{
				return info->page->GetGuid() == guid;
			};

			return JCUtil::GetIndex(pageSizeTInfo, pageEqualPtr, guid);
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
				//JEditorPageShareData::ClearPageData(clearEvStruct->pageType);
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
			case J_EDITOR_EVENT::MAXIMIZE_PAGE:
			{
				MaximizePage(static_cast<JEditorMaximizePageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::MINIMIZE_PAGE:
			{
				MinimizePage(static_cast<JEditorMinimizePageEvStruct*>(eventStruct));
				break;
			}
			case J_EDITOR_EVENT::PREVIOUS_SIZE_PAGE:
			{
				PreviousSizePage(static_cast<JEditorPreviousSizePageEvStruct*>(eventStruct));
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