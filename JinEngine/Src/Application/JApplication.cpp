#include"JApplication.h"   
#include"JApplicationEngine.h" 
#include"JApplicationEnginePrivate.h" 
#include"JApplicationProject.h" 
#include"JApplicationProjectPrivate.h" 
#include"JApplicationCloseType.h"
#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"
#include"../Graphic/JGraphic.h" 
#include"../Graphic/JGraphicPrivate.h" 
#include"../Object/Resource/JResourceManager.h"  
#include"../Object/Resource/JResourceManagerPrivate.h"  
#include"../Core/Reflection/JReflectionInfoPrivate.h"
#include"../Core/Identity/JIdentifier.h"
#include"../Core/Time/JGameTimer.h"  
#include"../Core/Time/JStopWatch.h"  
#include"../Core/Func/Functor/JFunctor.h"  
#include"../Core/Event/JEventListener.h"
#include"../Core/Threading/JThreadManagerPrivate.h"
#include"../Utility/JCommonUtility.h" 
#include"../Editor/JEditorManager.h" 
#include"../Window/JWindowEventType.h"
   
namespace JinEngine
{
	namespace Application
	{
		namespace
		{
			using WindowAppAccess = Window::JWindowPrivate::ApplicationAccess;
			using ThreadManagerAccess = Core::JThreadManagerPrivate::ApplicationInterface;
			using ResourceManagerAppAccess = JResourceManagerPrivate::ApplicationAccess;
			using RefelectionAppAccess = Core::JReflectionInfoPrivate::ApplicationInterface;
			using EngineAppAccess = JApplicationEnginePrivate::AppAccess;
			using ProjectAppAccess = JApplicationProjectPrivate::AppAccess;
			using ProjectLifeInterface = JApplicationProjectPrivate::LifeInterface;
			using GraphicAppAccess = Graphic::JGraphicPrivate::AppAccess;
			using GraphicCommandInterface = Graphic::JGraphicPrivate::CommandInterface;
		}

		class JApplication::JApplicationImpl : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
		{
		private:
			using LoadProjectF = Core::JMFunctorType <JApplication::JApplicationImpl, void>;
			using StoreProjectF = Core::JMFunctorType <JApplication::JApplicationImpl, void>;
			using SetAppStateF = JApplicationProjectPrivate::SetAppStateF;
		public:
			const size_t guid;
			Editor::JEditorManager editorManager;
		public:
			JApplicationImpl(HINSTANCE hInstance, const char* commandLine, const size_t guid)
				:guid(guid)
			{ 
				RefelectionAppAccess::Initialize();
				EngineAppAccess::Initialize();
				ThreadManagerAccess::Initialize();

				auto loadProjectF = std::make_unique<LoadProjectF::Functor>(&JApplicationImpl::LoadProject, this);
				auto storeProjectF = std::make_unique<StoreProjectF::Functor>(&JApplicationImpl::StoreProject, this);
				auto setAppStateF = std::make_unique<SetAppStateF>(&JApplicationEnginePrivate::AppAccess::SetApplicationState);

				ProjectAppAccess::RegisterFunctor(std::move(loadProjectF), std::move(storeProjectF), std::move(setAppStateF));
				WindowAppAccess::Initialize(hInstance, std::make_unique<WindowAppAccess::CloseConfirmF>(&JApplicationImpl::CloseApp, this));
			}
			~JApplicationImpl()
			{
				ThreadManagerAccess::Clear();
				RefelectionAppAccess::Clear();
			}
		public: 
			void RunProjectSelector()
			{
				AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);

				WindowAppAccess::OpenProjecSelectorWindow();
				GraphicAppAccess::Initialize();
				ResourceManagerAppAccess::Initialize();
				ResourceManagerAppAccess::LoadSelectorResource();

				editorManager.Initialize();
				editorManager.OpenProjectSelector();

				JEngineTimer::Data().Start();
				JEngineTimer::Data().Reset();

				while (true)
				{
					std::optional<int> encode = WindowAppAccess::ProcessMessages();
					if (encode.has_value())
						break;
					 
					Core::JGameTimer::UpdateAllTimer();
					GraphicAppAccess::UpdateWait();
					RefelectionAppAccess::Update();
					ThreadManagerAccess::Update();
					GraphicAppAccess::UpdateGuiBackend();
					editorManager.Update();
					GraphicAppAccess::DrawGui();
					 
					if (ProjectAppAccess::CanStartProject())
					{
						GraphicAppAccess::UpdateWait();
						WindowAppAccess::CloseWindow();
					}
				} 
			}
			void RunEngine()
			{
				if (!ProjectAppAccess::Initialize())
					return;

				AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);
			 
				WindowAppAccess::OpenEngineWindow(); 
				GraphicAppAccess::Initialize();
				ResourceManagerAppAccess::Initialize();
				ResourceManagerAppAccess::LoadProjectResource();

				editorManager.Initialize();
				editorManager.OpenProject();

				JEngineTimer::Data().Start();
				JEngineTimer::Data().Reset();
				while (true)
				{
					std::optional<int> encode = WindowAppAccess::ProcessMessages();
					if (encode.has_value())
						break;

					Core::JGameTimer::UpdateAllTimer();
					GraphicAppAccess::UpdateWait(); 
					RefelectionAppAccess::Update();
					ThreadManagerAccess::Update();
					GraphicAppAccess::UpdateGuiBackend();
					editorManager.Update(); 
					GraphicAppAccess::UpdateFrame();
					GraphicAppAccess::DrawScene();

					if (ProjectAppAccess::CanEndProject())
					{
						GraphicAppAccess::UpdateWait();
						WindowAppAccess::CloseWindow();
						ProjectAppAccess::CloseProject();
					}
				}
			}
		public:
			void CloseApp()
			{
				const J_APPLICATION_STATE appState = JApplicationEngine::GetApplicationState();
				if (appState == J_APPLICATION_STATE::PROJECT_SELECT)
					WindowAppAccess::CloseWindow();
				else if (appState == J_APPLICATION_STATE::EDIT_GAME)
				{
					ProjectAppAccess::BeginCloseProject();
					editorManager.PressMainWindowCloseButton();
				}
				else
					;
			}
		public:
			void StoreProject()
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
					ResourceManagerAppAccess::StoreProjectResource();
			}
			void LoadProject()
			{
				std::wstring dirPath;
				if (JWindow::SelectDirectory(dirPath, L"please, select project root directory"))
				{ 
					if (dirPath == JApplicationProject::RootPath())
						return;

					std::unique_ptr<JApplicationProjectInfo> pInfo;
					JApplicationProjectInfo* existingInfo = JApplicationProject::GetProjectInfo(dirPath);
					if (existingInfo != nullptr)
						pInfo = existingInfo->GetUnique();
					else
						pInfo = ProjectLifeInterface::MakeProjectInfo(dirPath);
					if (pInfo != nullptr)
					{
						ProjectAppAccess::BeginLoadOtherProject();
						ProjectLifeInterface::SetNextProjectInfo(std::move(pInfo));
						CloseApp();
						//JWindow::Instance().AppInterface()->CloseWindow();
						//if (!JApplicationProject::SetStartNewProjectTrigger())
						//	MessageBox(0, L"Fail start project", 0, 0);
					}
					else
						MessageBox(0, L"Invalid project path", 0, 0);
				}
			}
		public:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final
			{
				if (senderGuid == guid)
					return;

				if (eventType == Window::J_WINDOW_EVENT::WINDOW_CLOSE)
				{
					if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::PROJECT_SELECT)
					{
						GraphicCommandInterface::FlushCommandQueue();
						editorManager.Clear();
						ResourceManagerAppAccess::Terminate();
						GraphicAppAccess::Clear();
					}
					else if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
					{
						GraphicCommandInterface::FlushCommandQueue();
						GraphicAppAccess::WriteLastRsTexture();
						editorManager.StorePage();
						editorManager.Clear();
						ResourceManagerAppAccess::Terminate();
						GraphicAppAccess::Clear();
					}
				}
			}
		};

		JApplication::JApplication(HINSTANCE hInstance, const char* commandLine)
			:impl(std::make_unique<JApplicationImpl>(hInstance, commandLine, Core::MakeGuid()))
		{}
		JApplication::~JApplication()
		{
		}
		void JApplication::Run()
		{
			impl->RunProjectSelector();
			while (ProjectAppAccess::CanStartProject())
				impl->RunEngine();
		}
	}
}
