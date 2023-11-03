#include"JMain.h"   
#include"../Application/JApplicationEngine.h" 
#include"../Application/JApplicationEnginePrivate.h" 
#include"../Application/JApplicationProject.h" 
#include"../Application/JApplicationProjectPrivate.h"  
#include"../Window/JWindow.h"
#include"../Window/JWindowPrivate.h"
#include"../Graphic/JGraphic.h" 
#include"../Graphic/Adapter/JGraphicAdapter.h"  
#include"../Graphic/Adapter/JDx12Adaptee.h" 
#include"../Graphic/Gui/JGuiBackendDataAdapter.h" 
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
#include"../Core/Module/JModuleManager.h"
#include"../Core/Module/JModuleManagerPrivate.h"
#include"../Core/Plugin/JPluginManager.h"
#include"../Core/Plugin/JPluginManagerPrivate.h"
#include"../Core/Utility/JCommonUtility.h" 
#include"../Core/Reflection/JTypeBase.h"
#include"../Editor/JEditorManager.h" 
#include"../Editor/Interface/JEditorProjectInterface.h" 
#include"../Editor/Gui/Data/ImGui/JImGuiPrivateData.h"
#include"../Editor/Gui/Data/ImGui/JImGuiInitData.h"
#include"../Editor/Gui/Adapter/JGuiBehaviorAdapter.h" 
#include"../Editor/Gui/Adapter/ImGui/Dx/JDx12ImGuiAdaptee.h" 
#include"../Editor/Gui/Adapter/ImGui/Dx/JDx12ImGuiBackendDataAdaptee.h" 
#include"../Window/JWindowEventType.h"

//Debug
#include"../Develop/Debug/JDevelopDebugMain.h"
#define SKIP_SELECTOR
#ifdef SKIP_SELECTOR
#define _SKIP_SELECTOR
#endif

namespace JinEngine
{
	namespace
	{
		using WindowMainAccess = Window::JWindowPrivate::MainAccess;
		using ThreadManagerAccess = Core::JThreadManagerPrivate::MainAccess;
		using ResourceManagerMainAccess = JResourceManagerPrivate::MainAccess;
		using RefelectionMainAccess = Core::JReflectionInfoPrivate::MainAccess;
		using EngineMainAccess = JApplicationEnginePrivate::MainAccess;
		using ProjectMainAccess = JApplicationProjectPrivate::MainAccess;
		using ProjectLifeInterface = JApplicationProjectPrivate::LifeInterface;
		using ProjectIOInterface = JApplicationProjectPrivate::IOInterface;
		using GraphicMainAccess = Graphic::JGraphicPrivate::MainAccess;
		using ModuleMainAccess = Core::JModuleManagerPrivate::MainAccess;
		using PluginMainAccess = Core::JPluginManagerPrivate::MainAccess; 
	}

	class JMain::JMainImpl : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
	{ 
	private:
		using SetAppStateF = JApplicationProjectPrivate::SetAppStateF;
	private:
		enum class END_FRAME_EVENT
		{
			NONE,
			RELOAD_PROJECT
		};
	public:
		const size_t guid;
		Editor::JEditorManager editorManager;
	public:
		END_FRAME_EVENT edFrameEv = END_FRAME_EVENT::NONE; 
	public:
		JMainImpl(HINSTANCE hInstance, const char* commandLine, const size_t guid)
			:guid(guid)
		{
			RefelectionMainAccess::Initialize();
			EngineMainAccess::Initialize();
			ThreadManagerAccess::Initialize();
			_JModuleManager::Instance().LoadModule(JApplicationEngine::SolutionPath());
			_JPluginManager::Instance().LoadPlugin(JApplicationEngine::SolutionPath());

			auto setAppStateF = std::make_unique<SetAppStateF>(&JApplicationEnginePrivate::MainAccess::SetApplicationState);
			ProjectMainAccess::RegisterFunctor(std::move(setAppStateF));
			WindowMainAccess::Initialize(hInstance, std::make_unique<WindowMainAccess::CloseConfirmF>(&JMainImpl::CloseAppProcess, this));
		}
		~JMainImpl()
		{
			PluginMainAccess::Clear();
			ModuleMainAccess::Clear();
			ThreadManagerAccess::Clear();
			RefelectionMainAccess::Clear();
		}
	public:
		void RunProjectSelector()
		{
			AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);

			Develop::JDevelopDebugMain::Initialize();
			WindowMainAccess::OpenProjecSelectorWindow();
			GraphicMainAccess::Initialize(CreateGraphicAdapter(), CreateGuiBackendDataAdapter(), editorManager.GetBackendInterface());
			ResourceManagerMainAccess::Initialize();
			ResourceManagerMainAccess::LoadSelectorResource();

			editorManager.Initialize(CreateGuiBehaviorAdapter());
			editorManager.OpenProjectSelector(GraphicMainAccess::GetGuiInitData(), CreateEditorProjectInterface());

			JEngineTimer::Data().Start();
			JEngineTimer::Data().Reset();

			while (true)
			{ 
				std::optional<int> encode = WindowMainAccess::ProcessMessages();
				if (encode.has_value())
					break;

				Core::JGameTimer::UpdateAllTimer();
				GraphicMainAccess::UpdateWait();
				RefelectionMainAccess::Update();
				ThreadManagerAccess::Update();  
				editorManager.Update();
				GraphicMainAccess::Draw(false);

				if (edFrameEv != END_FRAME_EVENT::NONE)
					edFrameEv = END_FRAME_EVENT::NONE;

				if (ProjectMainAccess::CanStartProject())
				{
					GraphicMainAccess::UpdateWait();
					WindowMainAccess::CloseWindow();
				} 
			}
		}
		void RunEngine()
		{
			if (!ProjectMainAccess::Initialize())
				return;

			AddEventListener(*JWindow::EvInterface(), guid, Window::J_WINDOW_EVENT::WINDOW_CLOSE);
			_JModuleManager::Instance().LoadModule(JApplicationProject::RootPath());
			_JPluginManager::Instance().LoadPlugin(JApplicationProject::RootPath());
			Develop::JDevelopDebugMain::Initialize();
			WindowMainAccess::OpenEngineWindow();
			GraphicMainAccess::Initialize(CreateGraphicAdapter(), CreateGuiBackendDataAdapter(), editorManager.GetBackendInterface());
			ResourceManagerMainAccess::Initialize();
			ResourceManagerMainAccess::LoadProjectResource();
			editorManager.Initialize(CreateGuiBehaviorAdapter());
			editorManager.OpenProject(GraphicMainAccess::GetGuiInitData(), CreateEditorProjectInterface());
			JEngineTimer::Data().Start();
			JEngineTimer::Data().Reset();

			while (true)
			{
				std::optional<int> encode = WindowMainAccess::ProcessMessages();
				if (encode.has_value())
					break;

				Core::JGameTimer::UpdateAllTimer();
				GraphicMainAccess::UpdateWait();
				RefelectionMainAccess::Update();
				ThreadManagerAccess::Update();  
				editorManager.Update();
				GraphicMainAccess::UpdateFrame();
				GraphicMainAccess::Draw(true);

				if (edFrameEv != END_FRAME_EVENT::NONE)
				{
					switch (edFrameEv)
					{ 
					case JinEngine::JMain::JMainImpl::END_FRAME_EVENT::RELOAD_PROJECT:
					{
						ReLoadProject();				 
					}
					default:
						break;
					}
					edFrameEv = END_FRAME_EVENT::NONE;
				}
				if (ProjectMainAccess::CanEndProject())
				{
					GraphicMainAccess::UpdateWait();
					WindowMainAccess::CloseWindow();
					ProjectMainAccess::CloseProject();
				}
			}
		}
		void TryRunEngine()
		{
			ProjectIOInterface::LoadProjectList();
			std::unique_ptr<JApplicationProjectInfo> pInfo = JApplicationProject::GetProjectInfo(0)->CreateReplica();
			StartProject(std::move(pInfo));
			RunEngine();
		}
	private:
		void CloseAppProcess()
		{
			const J_APPLICATION_STATE appState = JApplicationEngine::GetApplicationState();
			if (appState == J_APPLICATION_STATE::PROJECT_SELECT)
				WindowMainAccess::CloseWindow();
			else if (appState == J_APPLICATION_STATE::EDIT_GAME)
			{
				ProjectMainAccess::BeginCloseProject();
				editorManager.PressMainWindowCloseButton();
			}
			else
				;
		}
	private:
		void ReLoadProject()
		{
			if (JApplicationProject::GetOpenProjectInfo() == nullptr)
				return;

			GraphicMainAccess::FlushCommandQueue();
			editorManager.StorePage();
			editorManager.Clear();
			ResourceManagerMainAccess::Terminate(false);
			GraphicMainAccess::FlushCommandQueue();

			ResourceManagerMainAccess::Initialize();
			ResourceManagerMainAccess::LoadProjectResource();
			editorManager.Initialize(CreateGuiBehaviorAdapter());
			editorManager.OpenProject(GraphicMainAccess::GetGuiInitData(), CreateEditorProjectInterface());
		}
		void LoadOtherProject()
		{
			std::wstring dirPath;
			if (JWindow::SelectDirectory(dirPath, L"please, select project root directory"))
			{
				if (dirPath == JApplicationProject::RootPath())
					return;

				std::unique_ptr<JApplicationProjectInfo> pInfo;
				JApplicationProjectInfo* existingInfo = JApplicationProject::GetProjectInfo(dirPath);
				if (existingInfo != nullptr)
					pInfo = existingInfo->CreateReplica();
				else
					pInfo = ProjectLifeInterface::MakeProjectInfo(dirPath);
				if (pInfo != nullptr)
				{
					//pInfo가 비정확할경우 project는 load되지않고 종료된다.
					ProjectMainAccess::BeginLoadOtherProject(std::move(pInfo));
					CloseAppProcess();
					//JWindow::Instance().MainAccess()->CloseWindow();
					//if (!JApplicationProject::SetStartNewProjectTrigger())
					//	MessageBox(0, L"Fail start project", 0, 0);
				}
				else
					MessageBox(0, L"Invalid project path", 0, 0);
			}
		}
		void LoadUnRegisteredProject(const std::wstring path)
		{ 
			std::unique_ptr<JApplicationProjectInfo> pInfo;
			JApplicationProjectInfo* existingInfo = JApplicationProject::GetProjectInfo(path);
			if (existingInfo != nullptr)
				pInfo = existingInfo->CreateReplica();
			else
				pInfo = ProjectLifeInterface::MakeProjectInfo(path);

			ProjectLifeInterface::SetNextProjectInfo(std::move(pInfo));
			if (!ProjectLifeInterface::SetStartNewProjectTrigger())
				MessageBox(0, L"Load Project Fail", 0, 0);
		}
		void StoreProject()
		{
			if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
				ResourceManagerMainAccess::StoreProjectResource();
		}
	private:
		void RequestReLoadProject()
		{
			if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
			{
				if (JApplicationProject::GetOpenProjectInfo() == nullptr)
					return;

				edFrameEv = END_FRAME_EVENT::RELOAD_PROJECT;
			}
		}
	private:
		void CreateProject(const std::wstring path, const std::string version)
		{ 
			ProjectLifeInterface::SetNextProjectInfo(ProjectLifeInterface::MakeProjectInfo(path, version));
			if (!ProjectLifeInterface::SetStartNewProjectTrigger())
				MessageBox(0, L"Create Project Fail", 0, 0);
		}
		void StartProject(std::unique_ptr<JApplicationProjectInfo>&& pInfo)
		{
			using ProjectLifeInterface = JApplicationProjectPrivate::LifeInterface;
			ProjectLifeInterface::SetNextProjectInfo(std::move(pInfo));
			if (!ProjectLifeInterface::SetStartNewProjectTrigger())
				MessageBox(0, L"Start Project Fail", 0, 0);
		}
	private:
		void CloseSelector()
		{
			GraphicMainAccess::FlushCommandQueue();
			editorManager.Clear();
			ResourceManagerMainAccess::Terminate(true);
			GraphicMainAccess::Clear();
			Develop::JDevelopDebugMain::Clear();
		}
		void CloseProject()
		{
			GraphicMainAccess::FlushCommandQueue();
			GraphicMainAccess::WriteLastRsTexture();
			editorManager.StorePage();
			editorManager.Clear();
			ResourceManagerMainAccess::Terminate(true);
			GraphicMainAccess::Clear();
			Develop::JDevelopDebugMain::Clear();
		}
	private:
		std::unique_ptr< Editor::JEditorProjectInterface> CreateEditorProjectInterface()
		{
			using EditProjectInterface = Editor::JEditorProjectInterface;
			auto loadProjectListF = std::make_unique<EditProjectInterface::LoadProjectListF::Functor>(&ProjectIOInterface::LoadProjectList);
			auto storeProjectListF = std::make_unique<EditProjectInterface::StoreProjectListF::Functor>(&ProjectIOInterface::StoreProjectList);
			auto reLoadProjectF = std::make_unique<EditProjectInterface::ReLoadProjectF::Functor>(&JMainImpl::RequestReLoadProject, this);
			auto loadOtherProjectF = std::make_unique<EditProjectInterface::LoadOtherProjectF::Functor>(&JMainImpl::LoadOtherProject, this);
			auto loadUnRegisteredProjectF = std::make_unique<EditProjectInterface::LoadUnRegisteredProjectF::Functor>(&JMainImpl::LoadUnRegisteredProject, this);
			auto storeProjectF = std::make_unique<EditProjectInterface::StoreProjectF::Functor>(&JMainImpl::StoreProject, this);
			auto createProjectF = std::make_unique<EditProjectInterface::CreateProjectF::Functor>(&JMainImpl::CreateProject, this);
			auto destroyProjectF = std::make_unique<EditProjectInterface::DestroyProjectF::Functor>(&ProjectLifeInterface::DestroyProject);
			auto startProjectF = std::make_unique<EditProjectInterface::StartProjectF::Functor>(&JMainImpl::StartProject, this);
			auto confirmCloseProjectF = std::make_unique<EditProjectInterface::ConfirmCloseProjectF::Functor>(&ProjectLifeInterface::ConfirmCloseProject);
			auto closeCloseProjectF = std::make_unique<EditProjectInterface::CloseCloseProjectF::Functor>(&ProjectLifeInterface::CancelCloseProject);

			return std::make_unique<Editor::JEditorProjectInterface>(std::move(loadProjectListF), std::move(storeProjectListF),
				std::move(reLoadProjectF),std::move(loadOtherProjectF), 
				std::move(loadUnRegisteredProjectF), std::move(storeProjectF),
				std::move(createProjectF), std::move(destroyProjectF),
				std::move(startProjectF), std::move(confirmCloseProjectF), 
				std::move(closeCloseProjectF));
		}
	public:
		void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final
		{
			if (senderGuid == guid)
				return;

			if (eventType == Window::J_WINDOW_EVENT::WINDOW_CLOSE)
			{
				if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::PROJECT_SELECT)
					CloseSelector();
				else if (JApplicationEngine::GetApplicationState() == J_APPLICATION_STATE::EDIT_GAME)
					CloseProject();
			}
		}
	private:
		std::unique_ptr<Graphic::JGraphicAdapter> CreateGraphicAdapter()
		{
			std::unique_ptr<Graphic::JGraphicAdapter> adapter = std::make_unique<Graphic::JGraphicAdapter>();
			adapter->AddAdaptee(std::make_unique<Graphic::JDx12Adaptee>());
			return std::move(adapter);
		}
		std::unique_ptr<Graphic::JGuiBackendDataAdapter> CreateGuiBackendDataAdapter()
		{
			std::unique_ptr<Graphic::JGuiBackendDataAdapter> adapter = std::make_unique<Graphic::JGuiBackendDataAdapter>();
			adapter->AddAdaptee(std::make_unique<Editor::JDx12ImGuiBackendDataAdaptee>());
			return std::move(adapter);
		}
		std::unique_ptr<Editor::JGuiBehaviorAdapter> CreateGuiBehaviorAdapter()
		{
			std::unique_ptr<Editor::JGuiBehaviorAdapter> adapter = std::make_unique<Editor::JGuiBehaviorAdapter>();
			adapter->AddAdaptee(std::make_unique<Editor::JDx12ImGuiAdaptee>());
			return std::move(adapter);
		}
	};

	JMain::JMain(HINSTANCE hInstance, const char* commandLine)
		:impl(std::make_unique<JMainImpl>(hInstance, commandLine, Core::MakeGuid()))
	{}
	JMain::~JMain(){}
	void JMain::Run()
	{
#ifdef _SKIP_SELECTOR
		impl->TryRunEngine();
#else
		impl->RunProjectSelector();
		while (ProjectMainAccess::CanStartProject())
			impl->RunEngine();
#endif
	}
}

