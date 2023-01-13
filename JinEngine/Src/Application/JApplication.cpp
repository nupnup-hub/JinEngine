#include"JApplication.h"   
#include"JApplicationVariable.h" 
#include"JApplicationCloseType.h"
#include"../Window/JWindows.h"
#include"../Graphic/JGraphic.h" 
#include"../Object/Resource/JResourceManager.h"  
#include"../Core/Reflection/JReflectionInfo.h"
#include"../Core/Identity/JIdentifier.h"
#include"../Core/Time/JGameTimer.h"  
#include"../Utility/JCommonUtility.h"
#include"../Debug/JDebugTimer.h"

namespace JinEngine
{
	namespace Application
	{
		JApplication::JApplication(HINSTANCE hInstance, const char* commandLine)
			:guid(Core::MakeGuid())
		{		
			JApplicationVariable::Initialize();
			JApplicationVariable::RegisterFunctor(this, &JApplication::StoreProject, &JApplication::LoadProject);
			
			using CloseConfirmF = Window::JWindowAppInterface::CloseConfirmF;
			JWindow::Instance().AppInterface()->Initialize(hInstance, std::make_unique<CloseConfirmF::Functor>(&JApplication::CloseApp, this));
		}
		JApplication::~JApplication()
		{}
		void JApplication::Run()
		{
			RunProjectSelector();
			while (JApplicationProject::CanStartProject())
				RunEngine();
		}
		void JApplication::RunProjectSelector()
		{
			AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);

			JWindow::Instance().AppInterface()->OpenProjecSelectorWindow();
			JGraphic::Instance().AppInterface()->Initialize();
			JResourceManager::Instance().AppInterface()->Initialize();
			JResourceManager::Instance().AppInterface()->LoadSelectorResource();

			editorManager.Initialize();
			editorManager.OpenProjectSelector();

			JGameTimer::Instance().Start();
			JGameTimer::Instance().Reset();
			JGameTimer::Instance().Tick();  
			while (true)
			{
				std::optional<int> encode = JWindow::Instance().AppInterface()->ProcessMessages();
				if (encode.has_value())
					break;

				JGameTimer::Instance().Tick();
				CalculateFrame();

				JGraphic::Instance().AppInterface()->UpdateWait();
				JGraphic::Instance().AppInterface()->StartFrame();
				JGraphic::Instance().AppInterface()->DrawProjectSelector();
				editorManager.Update();
				JGraphic::Instance().AppInterface()->EndFrame();

				if (JApplicationProject::CanStartProject())
					JWindow::Instance().AppInterface()->CloseWindow();
			}
		}
		void JApplication::RunEngine()
		{
			if (!JApplicationProject::Initialize())
				return;

			AddEventListener(*JWindow::Instance().EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);
			JWindow::Instance().AppInterface()->OpenEngineWindow();
			JGraphic::Instance().AppInterface()->Initialize();

			JResourceManager::Instance().AppInterface()->Initialize();
			JResourceManager::Instance().AppInterface()->LoadProjectResource();

			editorManager.Initialize();
			editorManager.OpenProject();

			JGameTimer::Instance().Start();
			JGameTimer::Instance().Reset();
			JGameTimer::Instance().Tick();
			 
			while (true)
			{
				std::optional<int> encode = JWindow::Instance().AppInterface()->ProcessMessages();					 
				if (encode.has_value())
					break;

				JGameTimer::Instance().Tick();
				CalculateFrame();
				Core::JDebugTimer::StartGameTimer();
				JGraphic::Instance().AppInterface()->UpdateWait();
				JGraphic::Instance().AppInterface()->StartFrame();
				editorManager.Update();
				JGraphic::Instance().AppInterface()->UpdateEngine();
				JGraphic::Instance().AppInterface()->DrawScene();
				JGraphic::Instance().AppInterface()->EndFrame();

				Core::JDebugTimer::StopGameTimer(); 
				Core::JDebugTimer::RecordTime(guid);
				JApplicationVariable::ExecuteAppCommand();
				if (JApplicationProject::CanEndProject())
					JWindow::Instance().AppInterface()->CloseWindow();
			}
		}
		void JApplication::CalculateFrame()
		{ 
			frameCnt++;
			if ((JGameTimer::Instance().TotalTime() - timeElapsed) >= 1.0f)
			{
				float fps = (float)frameCnt; // fps = frameCnt / 1
				float mspf = 1000.0f / fps;
				std::wstring title = L"JinEngine";
				if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
					title += L"    Project: " + JApplicationVariable::GetActivatedProjectName();

				auto tResult = Core::JDebugTimer::GetTimeResult(guid);
				std::wstring fpsWstr = std::to_wstring(fps);
				std::wstring mspfWstr = std::to_wstring(mspf);
				std::wstring windowText = title +
					L"    fps: " + fpsWstr +
					L"   mspf: " + mspfWstr +
					L"   sec: " + std::to_wstring(tResult.secondTime) +
					L"   ms: " + std::to_wstring(tResult.msTime) +
					L"   nano: " + std::to_wstring(tResult.nanoTime);

				SetWindowText(JWindow::Instance().HandleInterface()->GetHandle(), windowText.c_str());
				// Reset for next average.
				frameCnt = 0;
				timeElapsed += 1.0f; 
			}
		}
		void JApplication::CloseApp()
		{
			const J_APPLICATION_STATE appState = JApplicationVariable::GetApplicationState();
			if (appState == J_APPLICATION_STATE::PROJECT_SELECT)
				JWindow::Instance().AppInterface()->CloseWindow();
			else if (appState == J_APPLICATION_STATE::EDIT_GAME)
				editorManager.PressMainWindowCloseButton();
			else
				;
		}
		void JApplication::StoreProject()
		{
			if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				JResourceManager::Instance().AppInterface()->StoreProjectResource();
		}
		void JApplication::LoadProject()
		{
			std::wstring dirpath;
			if (JWindow::Instance().SelectDirectory(dirpath, L"please, select project root directory"))
			{
				if (dirpath == JApplicationVariable::GetActivatedProjectPath())
					return;

				std::unique_ptr<JApplicationProject::JProjectInfo> pInfo = JApplicationProject::MakeProjectInfo(dirpath);
				if (pInfo != nullptr)
				{
					JWindow::Instance().AppInterface()->CloseWindow();
					JApplicationProject::SetNextProjectInfo(std::move(pInfo));
					if (!JApplicationProject::StartNewProject())
						MessageBox(0, L"Fail start project", 0, 0);
				}
				else
					MessageBox(0, L"Invalid project path", 0, 0);
			}
		}
		void JApplication::OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)
		{
			if (senderGuid == guid)
				return;

			if (eventType == Window::J_WINDOW_EVENT::WINDOW_CLOSE)
			{
				if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::PROJECT_SELECT)
				{
					JGraphic::Instance().AppInterface()->FlushCommandQueue();
					editorManager.Clear();
					JResourceManager::Instance().AppInterface()->Terminate();
					JGraphic::Instance().AppInterface()->Clear();
				}
				else if (JApplicationVariable::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				{
					JGraphic::Instance().AppInterface()->FlushCommandQueue();
					editorManager.StorePage();
					editorManager.Clear();
					JResourceManager::Instance().AppInterface()->Terminate(); 
					JGraphic::Instance().AppInterface()->Clear();
				}			 
			}
		}
	}
}
