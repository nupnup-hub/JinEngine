#include"EditorManager.h"
#include"Utility/EditorUtility.h" 
#include"Event/EditorEventStruct.h"
#include"Menubar/EditorMenuBar.h"
#include"Page/EditorWindow.h"  
#include"Page/Main/EditorMainPage.h"
#include"Page/SkeletonaAssetSetting/EditorSkeletonAssetPage.h" 
#include"Page/Debug/DebugHelper.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Application/JApplicationVariable.h"
#include"../../Object/Resource/Scene/JScene.h"
#include"../JWindows.h"
#include<fstream>  
#include<io.h>   

//수정필요
//Dock Split시 이전에 Focus하고있던 Window가 offFocus되지않음
namespace JinEngine
{
	EditorManager::EditorManager()
		:editorManagerGuid(JCommonUtility::CalculateGuid("EditorManager"))
	{ 
		editorUtility = std::make_unique<EditorUtility>();
		imguiManager = std::make_unique<ImGuiManager>();
		DebugHelper::Initialize();
	}
	EditorManager::~EditorManager() {}
	void EditorManager::SetEditorBackend()
	{
		imguiManager->SetImGuiBackend();
	}
	void EditorManager::OpenProjectSelector()
	{
	}
	void EditorManager::OpenProject(const std::string& mainSceneName)
	{
		editorUtility->UpdateWindoeData();
		editorUtility->UpdateEditorTextSize();

		editorPage.push_back(std::make_unique<EditorMainPage>());
		editorPage.push_back(std::make_unique<EditorSkeletonAssetPage>());

		const std::string imguiTxt = JApplicationVariable::GetEnginePath() + "imgui.ini";
		bool hasImguiTxt = false;
		if (_access(imguiTxt.c_str(), 00) != -1)
			hasImguiTxt = true;

		for (uint i = 0; i < editorPage.size(); ++i)
		{
			editorPage[i]->Initialize(editorUtility.get(), allEditorWindows, hasImguiTxt);
			editorPageMap.emplace(editorPage[i]->GetGuid(), editorPage[i].get());
		}

		const uint allEditorWindowCount = (uint)allEditorWindows.size();
		for (uint i = 0; i < allEditorWindowCount; ++i)
			allEditorWindowsMap.emplace(allEditorWindows[i]->GetGuid(), allEditorWindows[i]);

		if (_access(GetEditorDataPath(mainSceneName).c_str(), 00) != -1)
			LoadPage(mainSceneName);
		else
		{
			opendEditorPage.push_back(editorPage[0].get());
			opendEditorPage[0]->Activate(editorUtility.get());
		}

		std::vector<EDITOR_EVENT> eventVector
		{
			EDITOR_EVENT::OPEN_PAGE, EDITOR_EVENT::CLOSE_PAGE,
			EDITOR_EVENT::OPEN_WINDOW, EDITOR_EVENT::CLOSE_WINDOW,
			EDITOR_EVENT::ACTIVATE_WINDOW, EDITOR_EVENT::DEACTIVATE_WINDOW,
		};
		this->AddEventListener(*editorUtility->EvInterface(), editorManagerGuid, eventVector);
	}
	void EditorManager::Update()
	{
		//ImGui에서 Graphic Resource 할당받기전에 이벤트처리
		OpenAndClosePage();
		OpenAndCloseWindow();
		ActivationWindow();

		editorUtility->UpdateWindoeData();
		editorUtility->UpdateEditorTextSize();

		uint8 pageCount = (uint8)opendEditorPage.size();
		for (uint8 i = 0; i < pageCount; ++i)
			opendEditorPage[i]->UpdatePage(editorUtility.get());
	}
	void EditorManager::Clear()
	{
		uint8 pageCount = (uint8)opendEditorPage.size();
		for (uint8 i = 0; i < pageCount; ++i)
			opendEditorPage[i]->DeActivate(editorUtility.get());
		 
		allEditorWindows.clear();
		allEditorWindowsMap.clear();
		editorPageMap.clear();
		opendEditorPage.clear();
		editorPage.clear();

		activationWindowQueue.clear();
		openCloseWindowQueue.clear();
		openClosePageQueue.clear();

		editorUtility.reset();
	}
	void EditorManager::LoadPage(const std::string& sceneName)
	{
		std::wstring guide;
		std::wifstream stream;
		stream.open(GetEditorDataPath(sceneName), std::ios::in);
		stream >> guide;
		for (int i = 0; i < editorPage.size(); ++i)
			editorPage[i]->LoadPage(stream, allEditorWindows, opendEditorPage, editorUtility.get());
		stream.close();
	}
	void EditorManager::StorePage(const std::string& sceneName)
	{
		std::wofstream stream;
		stream.open(GetEditorDataPath(sceneName), std::ios::out);
		stream << L"EditorPage: " << '\n';
		for (int i = 0; i < editorPage.size(); ++i)
		{
			editorPage[i]->StorePage(stream);
		}
		stream.close();
	}

	void EditorManager::OpenAndClosePage()
	{
		uint openCloseCount = (uint)openClosePageQueue.size();
		if (openCloseCount > 0)
		{
			for (uint i = 0; i < openCloseCount; ++i)
			{
				const EDITOR_EVENT editorEv = openClosePageQueue[i]->GetEventType();
				if (editorEv == EDITOR_EVENT::OPEN_PAGE)
				{
					EditorOpenPageEvStruct* opPageStruct = dynamic_cast<EditorOpenPageEvStruct*>(openClosePageQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(opPageStruct->openPageName);
					auto page = editorPageMap.find(guid);
					if (page != editorPageMap.end())
					{
						if (!page->second->IsOpen())
						{
							page->second->Open(editorUtility.get());
							page->second->SetFront(editorUtility.get());
							page->second->Activate(editorUtility.get());
							opendEditorPage.push_back(page->second);
						}
					}
				}
				else if (editorEv == EDITOR_EVENT::CLOSE_PAGE)
				{
					EditorClosePageEvStruct* closePageStruct = dynamic_cast<EditorClosePageEvStruct*>(openClosePageQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(closePageStruct->closePageName);
					auto page = editorPageMap.find(guid);
					if (page != editorPageMap.end())
					{
						if (page->second->IsOpen())
						{
							page->second->DeActivate(editorUtility.get());
							page->second->OffFront(editorUtility.get());
							page->second->Close(editorUtility.get());
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
			}
			openClosePageQueue.clear();
		}
	}
	void EditorManager::OpenAndCloseWindow()
	{
		uint openCloseCount = (uint)openCloseWindowQueue.size();
		if (openCloseCount > 0)
		{
			for (uint i = 0; i < openCloseCount; ++i)
			{
				const EDITOR_EVENT editorEv = openCloseWindowQueue[i]->GetEventType();
				if (editorEv == EDITOR_EVENT::OPEN_WINDOW)
				{
					EditorOpenWindowEvStruct* opWindowStruct = dynamic_cast<EditorOpenWindowEvStruct*>(openCloseWindowQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(opWindowStruct->openWindowName);
					auto window = allEditorWindowsMap.find(guid);
					if (window != allEditorWindowsMap.end())
					{
						auto ownerPage = editorPageMap.find(window->second->GetOwnerPageGuid());
						if (ownerPage != editorPageMap.end())
							ownerPage->second->OpenWindow(window->second, editorUtility.get());
					}
				}
				else if (editorEv == EDITOR_EVENT::CLOSE_WINDOW)
				{
					EditorCloseWindowEvStruct* opWindowStruct = dynamic_cast<EditorCloseWindowEvStruct*>(openCloseWindowQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(opWindowStruct->closeWindowName);
					auto window = allEditorWindowsMap.find(guid);
					if (window != allEditorWindowsMap.end())
					{
						auto ownerPage = editorPageMap.find(window->second->GetOwnerPageGuid());
						if (ownerPage != editorPageMap.end())
							ownerPage->second->CloseWindow(window->second, editorUtility.get());
					}
				}
			}
			openCloseWindowQueue.clear();
		}
	}
	void EditorManager::ActivationWindow()
	{
		uint activationCount = (uint)activationWindowQueue.size();
		if (activationCount > 0)
		{
			for (uint i = 0; i < activationCount; ++i)
			{
				const EDITOR_EVENT editorEv = activationWindowQueue[i]->GetEventType();
				if (editorEv == EDITOR_EVENT::ACTIVATE_WINDOW)
				{
					EditorActWindowEvStruct* actWindowStruct = dynamic_cast<EditorActWindowEvStruct*>(activationWindowQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(actWindowStruct->actWindowName);
					auto window = allEditorWindowsMap.find(guid);
					if (window != allEditorWindowsMap.end())
					{
						auto ownerPage = editorPageMap.find(window->second->GetOwnerPageGuid());
						if (ownerPage != editorPageMap.end())
						{
							window->second->SetFront(editorUtility.get());
							window->second->Activate(editorUtility.get());
							if (actWindowStruct->actFocus)
								window->second->OnFocus(editorUtility.get());
						}
					}
				}
				else if (editorEv == EDITOR_EVENT::DEACTIVATE_WINDOW)
				{
					EditorDeActWindowEvStruct* actWindowStruct = dynamic_cast<EditorDeActWindowEvStruct*>(activationWindowQueue[i].get());
					const size_t guid = JCommonUtility::CalculateGuid(actWindowStruct->deActWindowName);
					auto window = allEditorWindowsMap.find(guid);
					if (window != allEditorWindowsMap.end())
					{
						auto ownerPage = editorPageMap.find(window->second->GetOwnerPageGuid());
						if (ownerPage != editorPageMap.end())
						{
							window->second->DeActivate(editorUtility.get());
							window->second->OffFront(editorUtility.get());
						}
					}
				}
			}
			activationWindowQueue.clear();
		}
	}
	std::string EditorManager::GetEditorDataPath(const std::string& sceneName)noexcept
	{
		return JApplicationVariable::GetProjectEditorResourcePath() + "\\" + sceneName + "_" + editorPageDataFileName;
	}
	void EditorManager::OnEvent(const size_t& senderGuid, const EDITOR_EVENT& eventType, EditorEventStruct* eventStruct)
	{
		if (senderGuid == editorManagerGuid || !eventStruct->PassDefectInspection())
			return;
		switch (eventType)
		{
		case JinEngine::EDITOR_EVENT::OPEN_PAGE:
			openClosePageQueue.push_back(std::make_unique<EditorOpenPageEvStruct>(*dynamic_cast<EditorOpenPageEvStruct*>(eventStruct)));
			break;
		case JinEngine::EDITOR_EVENT::CLOSE_PAGE:
			openClosePageQueue.push_back(std::make_unique<EditorClosePageEvStruct>(*dynamic_cast<EditorClosePageEvStruct*>(eventStruct)));
			break;
		case JinEngine::EDITOR_EVENT::OPEN_WINDOW:
			openCloseWindowQueue.push_back(std::make_unique<EditorOpenWindowEvStruct>(*dynamic_cast<EditorOpenWindowEvStruct*>(eventStruct)));
			break;
		case JinEngine::EDITOR_EVENT::CLOSE_WINDOW:
			openCloseWindowQueue.push_back(std::make_unique<EditorCloseWindowEvStruct>(*dynamic_cast<EditorCloseWindowEvStruct*>(eventStruct)));
			break;
		case JinEngine::EDITOR_EVENT::ACTIVATE_WINDOW:
			activationWindowQueue.push_back(std::make_unique<EditorActWindowEvStruct>(*dynamic_cast<EditorActWindowEvStruct*>(eventStruct)));
			break;
		case JinEngine::EDITOR_EVENT::DEACTIVATE_WINDOW:
			activationWindowQueue.push_back(std::make_unique<EditorDeActWindowEvStruct>(*dynamic_cast<EditorDeActWindowEvStruct*>(eventStruct)));
			break;
		default:
			break;
		}
	}
}