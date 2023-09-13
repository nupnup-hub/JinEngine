#include"JEditorSkeletonPage.h"
#include"Window/JAvatarEditor.h"  
#include"../JEditorPageShareData.h"
#include"../JEditorAttribute.h"  
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../../Gui/JGui.h"   
#include"../../Menubar/JEditorMenuBar.h"
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/Component/JComponentCreator.h"
#include"../../../Object/Resource/JResourceManager.h"  
#include"../../../Object/Resource/Mesh/JSkinnedMeshGeometry.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JSceneManager.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"   
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Identity/JIdenCreator.h"
#include"../../../Core/Math/JVectorExtend.h"
#include"../../../Application/JApplicationProject.h" 

namespace JinEngine
{
	namespace Editor
	{
		//const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, std::unique_ptr<JEditorAttribute> attribute
		JEditorSkeletonPage::JEditorSkeletonPage()
			:JEditorPage("SkeletonAssetPage",
				std::make_unique<JEditorAttribute>(),	
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK, 
					J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING, 
					J_EDITOR_PAGE_SUPPORT_WINDOW_MAXIMIZE,
					J_EDITOR_PAGE_SUPPORT_WINDOW_MINIMIZE,
					J_EDITOR_PAGE_REQUIRE_INIT_OBJECT))
		{
			constexpr uint memberWindowCount = 4;
			std::vector<std::string> windowNames
			{
				"Skeleton Explorer##SkeletonAssetPage",
				"Avatar Editor##SkeletonAssetPage",
				"Avatar Observer##SkeletonAssetPage",
				"Avatar Detail##SkeletonAssetPage"
			};
			std::vector<std::unique_ptr<JEditorAttribute>> windowAttributes(memberWindowCount);
			for (uint i = 0; i < memberWindowCount; ++i)
				windowAttributes[i] = std::make_unique<JEditorAttribute>();

			//수정필요
			std::vector<J_OBSERVER_SETTING_TYPE> settingType
			{
				J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE
			};

			J_EDITOR_WINDOW_FLAG defaultFlag = J_EDITOR_WINDOW_SUPPORT_WINDOW_CLOSING;
			J_EDITOR_WINDOW_FLAG dockFlag = Core::AddSQValueEnum(defaultFlag, J_EDITOR_WINDOW_SUPROT_DOCK);

			explorer = std::make_unique<JObjectExplorer>(windowNames[0], std::move(windowAttributes[0]), GetPageType(), dockFlag);
			avatarEdit = std::make_unique<JAvatarEditor>(windowNames[1], std::move(windowAttributes[1]), GetPageType(), dockFlag);
			avatarObserver = std::make_unique<JSceneObserver>(windowNames[2], std::move(windowAttributes[2]), GetPageType(), dockFlag, settingType);
			avatarDetail = std::make_unique<JObjectDetail>(windowNames[3], std::move(windowAttributes[3]), GetPageType(), dockFlag);

			std::vector<JEditorWindow*> windows
			{
				explorer.get(),
				avatarEdit.get(),
				avatarObserver.get(),
				avatarDetail.get()
			};
			AddWindow(windows); 
		}
		JEditorSkeletonPage::~JEditorSkeletonPage()
		{}
		J_EDITOR_PAGE_TYPE JEditorSkeletonPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::SKELETON_SETTING;
		}
		void JEditorSkeletonPage::SetInitWindow()
		{
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				CloseWindow(GetOpenWindow(i));

			OpenWindow(explorer.get());
			OpenWindow(avatarEdit.get());
			OpenWindow(avatarObserver.get());
			OpenWindow(avatarDetail.get());

			currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->SetLastActivated(true);
		}
		void JEditorSkeletonPage::Initialize()
		{
			JEditorPage::Initialize();
			BuildMenuNode();
		}
		void JEditorSkeletonPage::UpdatePage()
		{
			if (setWndOptionOnce)
			{
				JGui::SetNextWindowSize(JGui::GetMainWorkSize(), J_GUI_CONDIITON_ONCE);
				JGui::SetNextWindowPos(JGui::GetMainWorkPos(), J_GUI_CONDIITON_ONCE);
				setWndOptionOnce = false;
			}
			//폰트 크기에 따라 페이지 윈도우 타이틀바에 사이즈가 달라진다.
			//반드시 page enter전에 font를 변경한다.
			JGui::SetFont(J_GUI_FONT_TYPE::MEDIUM);
			JGui::PushFont();

			//J_GUI_WINDOW_FLAG_ guiWindowFlag = J_GUI_WINDOW_FLAG_NO_NAV_INPUT |
			//J_GUI_WINDOW_FLAG_NO_BACKGROUND | J_GUI_WINDOW_FLAG_MENU_BAR |
			//J_GUI_WINDOW_FLAG_NO_TITLE_BAR | J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE;

			J_GUI_DOCK_NODE_FLAG_ dockspaceFlag = J_GUI_DOCK_NODE_FLAG_NO_WINDOW_MENU_BUTTON;
			J_GUI_WINDOW_FLAG_ guiWindowFlag = J_GUI_WINDOW_FLAG_MENU_BAR | J_GUI_WINDOW_FLAG_NO_SCROLL_BAR |
				J_GUI_WINDOW_FLAG_NO_COLLAPSE;

			EnterPage(guiWindowFlag); 
			const JVector2<float> pagePos = JGui::GetWindowPos();
			const JVector2<float> pageSize = JGui::GetWindowSize();
			if (HasDockNodeSpace())
				UpdateDockSpace(dockspaceFlag);
			else
			{
				BuildDockNode();
				setWndOptionOnce = true;
			} 
			menuBar->Update(true); 
			ClosePage();

			UpdateOpenWindow(); 
			JGui::PopFont(); 
			//UpdateOpenPopupWindow(pagePos, pageSize);
		}
		bool JEditorSkeletonPage::IsValidOpenRequest(const JUserPtr<Core::JIdentifier>& selectedObj)noexcept
		{
			return StuffSkeletonAssetData(selectedObj);
		}
		bool JEditorSkeletonPage::StuffSkeletonAssetData(const JUserPtr<Core::JIdentifier>& selectedObj)
		{
			if (!selectedObj.IsValid())
				return false;

			JUserPtr<JSkeletonAsset> newSkeletonAsset;
			if (!newSkeletonAsset.ConnnectChild(selectedObj))
				return false;

			skeleotnAsset = newSkeletonAsset; 
			Core::JTypeInstanceSearchHint hint = skeleotnAsset->GetModelHint();
			if (!hint.isValid)
				return false;
			
			//현재 skeleton은 skinned mesh서 파생되어 owner는 모두 skinned mesh이지만
			//미래에 skinned mesh이외에 skeleton을 파생할 수 있을경우 확인이 필요하다.
			JUserPtr<JMeshGeometry> mesh = Core::ConvertChildUserPtr<JMeshGeometry>(_JResourceManager::Instance().TryGetResourceUser(skeleotnAsset->GetModelHint()));
			if (mesh == nullptr)
				return false;

			const std::wstring sceneName = skeleotnAsset->GetName() + L"##_EditorScene";
			JUserPtr<JDirectory> dir = _JResourceManager::Instance().GetDirectory(JApplicationProject::DefaultResourcePath());

			const J_OBJECT_FLAG flag = Core::AddSQValueEnum(OBJECT_FLAG_DO_NOT_SAVE, OBJECT_FLAG_UNEDITABLE);
			JUserPtr<JScene> newScene = JICI::Create<JScene>(sceneName,
				Core::MakeGuid(),
				flag,
				JScene::GetDefaultFormatIndex(),
				dir,
				J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW);

			JUserPtr<JGameObject> meshObj = JICI::Create<JGameObject>(mesh->GetName(), Core::MakeGuid(), flag, newScene->GetRootGameObject());
			JCCI::CreateRenderItem(meshObj, mesh);

			std::vector<std::vector<uint8>> skeletonVec = skeleotnAsset->GetSkeletonTreeIndexVec();
			std::vector<JUserPtr<JGameObject>> skeletonTree((uint)skeletonVec.size());

			skeletonTree[0] = JICI::Create<JGameObject>(skeleotnAsset->GetJointName(0), Core::MakeGuid(), flag, meshObj);
			const uint jointCount = (uint)skeletonVec.size();
			for (uint i = 0; i < jointCount; ++i)
			{
				const uint childrenCount = (uint)skeletonVec[i].size();
				for (uint j = 0; j < childrenCount; ++j)
				{
					const uint index = skeletonVec[i][j];
					skeletonTree[index] = JICI::Create<JGameObject>(skeleotnAsset->GetJointName(index),
						Core::MakeGuid(),
						flag,
						skeletonTree[i]);
				}
			}
			if (avatarScene.IsValid())
				JObject::BeginDestroy(avatarScene.Release());

			avatarScene = newScene;
			explorer->Initialize(meshObj);
			avatarEdit->Initialize(skeleotnAsset);
			avatarObserver->Initialize(avatarScene, L"Skeleton Edit Cam");
			return true;
		}
		void JEditorSkeletonPage::DoSetClose()noexcept
		{
			if (avatarScene.IsValid())
				JObject::BeginDestroy(avatarScene.Release());
			JEditorPage::DoSetClose();
		}
		void JEditorSkeletonPage::DoActivate()noexcept
		{
			JEditorPage::DoActivate();
			ResourceEvListener::AddEventListener(*JResourceObject::EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void JEditorSkeletonPage::DoDeActivate()noexcept
		{
			ResourceEvListener::RemoveListener(*JResourceObject::EvInterface(), GetGuid());
			JEditorPage::DoDeActivate();
		}
		void JEditorSkeletonPage::StorePage(std::wofstream& stream)
		{
			JEditorPage::StorePage(stream);
		}
		void JEditorSkeletonPage::LoadPage(std::wifstream& stream)
		{
			JEditorPage::LoadPage(stream);
		}
		void JEditorSkeletonPage::BuildDockNode()
		{
			static constexpr int dockCount = 5;		//page(1) + window(4)
			std::vector<std::unique_ptr<JGuiDockBuildNode>> dockVec(dockCount);
			dockVec[0] = JGuiDockBuildNode::CreateRootNode(GetName(), GetDockNodeName());
			dockVec[1] = JGuiDockBuildNode::CreateNode(avatarObserver->GetName(), 1, 0, J_GUI_CARDINAL_DIR::LEFT, 0.7f);
			dockVec[2] = JGuiDockBuildNode::CreateNode(explorer->GetName(), 2, 0, J_GUI_CARDINAL_DIR::RIGHT, 0.3f);
			dockVec[3] = JGuiDockBuildNode::CreateNode(avatarEdit->GetName(), 3, 1, J_GUI_CARDINAL_DIR::DOWN, 0.35f);
			dockVec[4] = JGuiDockBuildNode::CreateNode(avatarDetail->GetName(), 4, 2, J_GUI_CARDINAL_DIR::DOWN, 0.5f);
			JGui::BuildDockHirechary(dockVec);
		}
		void JEditorSkeletonPage::BuildMenuNode()
		{
			std::unique_ptr<JEditorMenuNode> rootNode = std::make_unique<JEditorMenuNode>("Root", true, false, false);

			// root Child 
			std::unique_ptr<JEditorMenuNode> windowNode = std::make_unique<JEditorMenuNode>("Window", false, false, false, nullptr, rootNode.get());

			JEditorMenuNode* windowNodePtr = windowNode.get();
			menuBar = std::make_unique<JEditorMenuBar>(std::move(rootNode), false);
			menuBar->AddNode(std::move(windowNode));
			std::vector<JEditorWindow*> wndVec = GetWindowVec();

			const uint wndCount = (uint)wndVec.size();
			for (uint i = 0; i < wndCount; ++i)
			{
				std::unique_ptr<JEditorMenuNode> newNode = std::make_unique<JEditorMenuNode>(wndVec[i]->GetName(),
					false, true, false,
					wndVec[i]->GetOpenPtr(),
					windowNodePtr);

				auto openBind = std::make_unique<OpenEditorWindowF::CompletelyBind>(*GetOpenEditorWindowFunctorPtr(), *this, wndVec[i]->GetName());
				auto closeBind = std::make_unique<CloseEditorWindowF::CompletelyBind>(*GetCloseEditorWindowFunctorPtr(), *this, wndVec[i]->GetName());
				newNode->RegisterBindHandle(std::move(openBind), std::move(closeBind));
				menuBar->AddNode(std::move(newNode));
			}
		}
		void JEditorSkeletonPage::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (skeleotnAsset.IsValid() && jRobj->GetGuid() == skeleotnAsset->GetGuid())
				{
					skeleotnAsset.Clear();
					if (avatarScene.IsValid())
						JObject::BeginDestroy(avatarScene.Release());
				}
			}
		}
	}
}