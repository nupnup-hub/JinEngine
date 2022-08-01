#include"JApplication.h"   
#include"../Window/JWindows.h"
#include"../Object/Resource/JResourceManager.h"
#include"../Object/Component/JComponentDatabase.h"
#include"../Object/Resource/Scene/JScene.h"
#include"../Object/Component/Camera/JCamera.h"
#include"../Graphic/JGraphic.h" 
#include"../Core/GameTimer/JGameTimer.h" 

namespace JinEngine
{
	namespace Application
	{
		JApplication::JApplication(HINSTANCE hInstance, const char* commandLine)
		{
			JWindow::Instance().AppInterface()->Initialize(hInstance);
		}
		JApplication::~JApplication()
		{
		}
		void JApplication::Run()
		{ 
			JWindow::Instance().AppInterface()->OpenWindow();
			editorManager.SetEditorBackend();
			RunProjectSelector();
			if (applicationVar.applicationState == J_APPLICATION_STATE::EDIT_GAME)
				RunEngine();
		}
		void JApplication::RunProjectSelector()
		{
			JWindow::Instance().AppInterface()->SetProjectSelectorWindow();
			editorManager.OpenProjectSelector();
			projectSelector.Initialize();

			JGameTimer::Instance().Start();
			JGameTimer::Instance().Reset();
			JGameTimer::Instance().Tick();
			std::optional<int> encode;
			while (applicationVar.applicationState == J_APPLICATION_STATE::PROJECT_SELECT)
			{
				encode = JWindow::Instance().AppInterface()->ProcessMessages();
				if (encode.has_value())
					break;

				JGraphic::Instance().FrameInterface()->StartFrame();
				JGraphic::Instance().FrameInterface()->DrawProjectSelector();
				projectSelector.UpdateWindow(&applicationVar);
				JGraphic::Instance().FrameInterface()->EndFrame();

				JGameTimer::Instance().Tick();
				CalculateFrame();
			}
			JGraphic::Instance().FrameInterface()->FlushCommandQueue();
		}
		void JApplication::RunEngine()
		{
			JWindow::Instance().AppInterface()->SetEngineWindow();
			JComponentDatabase::Initialize();
			JResourceManager::Instance().LoadProjectResource();
			editorManager.OpenProject(JResourceManager::Instance().GetMainScene()->GetName());
			JGameTimer::Instance().Start();
			JGameTimer::Instance().Reset();
			JGameTimer::Instance().Tick();

			std::optional<int> encode;
			while (true)
			{
				encode = JWindow::Instance().AppInterface()->ProcessMessages();
				if (encode.has_value())
					break;

				JGraphic::Instance().FrameInterface()->StartFrame();
				JGraphic::Instance().FrameInterface()->UpdateWait();
				editorManager.Update();
				JGraphic::Instance().FrameInterface()->UpdateEngine();
				JGraphic::Instance().FrameInterface()->DrawScene();
				JGraphic::Instance().FrameInterface()->EndFrame();

				JGameTimer::Instance().Tick();
				CalculateFrame();
			}
			JGraphic::Instance().FrameInterface()->FlushCommandQueue();
			editorManager.StorePage(JResourceManager::Instance().GetMainScene()->GetName());
			editorManager.Clear();
			JResourceManager::Instance().Terminate();
		}
		void JApplication::CalculateFrame()
		{
			static int frameCnt = 0;
			static float timeElapsed = 0.0f;

			frameCnt++;
			if ((JGameTimer::Instance().TotalTime() - timeElapsed) >= 1.0f)
			{
				float fps = (float)frameCnt; // fps = frameCnt / 1
				float mspf = 1000.0f / fps;
				std::wstring title = L"JE_Engine";
				std::wstring fpsWstr = std::to_wstring(fps);
				std::wstring mspfWstr = std::to_wstring(mspf);
				std::wstring windowText = title +
					L"    fps: " + fpsWstr +
					L"   mspf: " + mspfWstr;

				SetWindowText(JWindow::Instance().HandleInterface()->GetHandle(), windowText.c_str());
				// Reset for next average.
				frameCnt = 0;
				timeElapsed += 1.0f;
			}
		}
	}
}
