#include"JEditorSkeletonPage.h"
#include"Window/JAvatarEditor.h" 
#include"../JEditorPageShareData.h"
#include"../JEditorAttribute.h"  
#include"../CommonWindow/View/JSceneObserver.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/GameObject/JGameObjectFactory.h"
#include"../../../Object/Component/JComponentFactoryUtility.h"
#include"../../../Object/Resource/JResourceManager.h"  
#include"../../../Object/Resource/Mesh/JSkinnedMeshGeometry.h"
#include"../../../Object/Resource/Scene/JScene.h"
#include"../../../Object/Resource/Scene/JSceneManager.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"  
#include"../../../Object/Resource/JResourceObjectFactory.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Application/JApplicationVariable.h"

namespace JinEngine
{
	namespace Editor
	{
		//const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, std::unique_ptr<JEditorAttribute> attribute
		JEditorSkeletonPage::JEditorSkeletonPage(bool hasMetadata)
			:JEditorPage("SkeletonAssetPage",
				std::make_unique<JEditorAttribute>(0.0f, 0.0f, 1.0f, 1.0f),
				Core::AddSQValueEnum(J_EDITOR_PAGE_SUPPORT_DOCK, J_EDITOR_PAGE_SUPPORT_WINDOW_CLOSING, J_EDITOR_PAGE_REQUIRE_INIT_OBJECT)),
			reqInitDockNode(!hasMetadata)
		{
			const uint memberWindowCount = 4;
			std::vector<std::string> windowNames
			{
				"Skeleton Explorer##SkeletonAssetPage",
				"Avatar Editor##SkeletonAssetPage",
				"Avatar Observer##SkeletonAssetPage",
				"Avatar Detail##SkeletonAssetPage"
			};
			std::vector<float> initWidthRateVec
			{
				0.25f, 0.25f, 0.55f, 0.2f
			};
			std::vector<float> initHeightRateVec
			{
				0.5f, 0.5f, 1, 1
			};
			std::vector<float> initPosXRateVec
			{
				0, 0, 0.25f, 0.7f,
			};
			std::vector<float> initPosYRateVec
			{
				0, 0.5f, 0, 0
			};
			std::vector<bool> openVec
			{
				true, true, true, true
			};;
			std::vector<std::unique_ptr< JEditorAttribute>> windowAttributes;
			for (uint i = 0; i < memberWindowCount; ++i)
			{
				windowAttributes.push_back(std::make_unique<JEditorAttribute>(initPosXRateVec[i],
					initPosYRateVec[i],
					initWidthRateVec[i],
					initHeightRateVec[i]));
			}

			//수정필요
			std::vector<J_OBSERVER_SETTING_TYPE> settingType
			{
				J_OBSERVER_SETTING_TYPE::OPTION_SPACE_SPATIAL,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_POS,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_ROT,
				J_OBSERVER_SETTING_TYPE::TOOL_EDIT_GOBJ_SCALE
			};

			explorer = std::make_unique<JObjectExplorer>(windowNames[0], std::move(windowAttributes[0]), GetPageType());
			avatarEdit = std::make_unique<JAvatarEditor>(windowNames[1], std::move(windowAttributes[1]), GetPageType());
			avatarObserver = std::make_unique<JSceneObserver>(windowNames[2], std::move(windowAttributes[2]), GetPageType(), settingType);
			avatarDetail = std::make_unique<JObjectDetail>(windowNames[3], std::move(windowAttributes[3]), GetPageType());

			std::vector<JEditorWindow*> windows
			{
				explorer.get(),
				avatarEdit.get(),
				avatarObserver.get(),
				avatarDetail.get()
			};
			AddWindow(windows);
			 
			JEditorPageShareData::RegisterPage(GetPageType(), &JEditorSkeletonPage::GetPageFlag, this);
		}
		JEditorSkeletonPage::~JEditorSkeletonPage()
		{
			JEditorPageShareData::UnRegisterPage(GetPageType());
		}
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

		}
		void JEditorSkeletonPage::UpdatePage()
		{
			JImGuiImpl::SetFont(J_EDITOR_FONT_TYPE::MEDIUM);
			JImGuiImpl::PushFont();

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
   
			ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y + ImGui::GetFrameHeight()));
			ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y - ImGui::GetFrameHeight()));

			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
			//ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
			//	ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoNavInputs |
				ImGuiWindowFlags_NoNavFocus |
				ImGuiWindowFlags_NoBackground | 
				ImGuiWindowFlags_NoScrollbar | 
				ImGuiWindowFlags_NoCollapse | 
				ImGuiWindowFlags_NoResize | 
				ImGuiWindowFlags_NoMove;

			//ImGuiWindowFlags windowFlag =
			//	ImGuiWindowFlags_NoScrollbar | 
			//	ImGuiWindowFlags_NoCollapse | 
			//	ImGuiWindowFlags_NoBackground;
			 
			EnterPage(windowFlag);
			if (reqInitDockNode)
			{
				BuildDockNode();
				reqInitDockNode = false;
			}
			UpdateDockSpace(dockspaceFlag);
			ClosePage(); 
			 
			uint currOpWndCount = GetOpenWindowCount();
			for (uint i = 0; i < currOpWndCount; ++i)
				GetOpenWindow(i)->UpdateWindow();  
			JImGuiImpl::PopFont();
			ImGui::PopStyleVar(2);
		}
		bool JEditorSkeletonPage::IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj)noexcept
		{
			return StuffSkeletonAssetData(selectedObj);
		}
		bool JEditorSkeletonPage::StuffSkeletonAssetData(const Core::JUserPtr<JObject>& selectedObj)
		{
			if (!selectedObj.IsValid())
				return false;

			Core::JUserPtr<JSkeletonAsset> skeleotnAsset;
			if (!skeleotnAsset.ConnnectChildUser(selectedObj))
				return false;

			auto isSameSkelLam = [](JMeshGeometry* mesh, size_t skelHash)
			{
				if (mesh->GetMeshGeometryType() == J_MESHGEOMETRY_TYPE::SKINNED)
				{
					JSkinnedMeshGeometry* skinnedMesh = static_cast<JSkinnedMeshGeometry*>(mesh);
					return skinnedMesh->GetSkeletonAsset()->GetSkeleton()->GetHash() == skelHash;
				}
				else
					return false;
			};
			using IsSameSkelFunctor = Core::JFunctor<bool, JMeshGeometry*, size_t>;
			JMeshGeometry* mesh = JResourceManager::Instance().GetResourceByCondition<JMeshGeometry, size_t>(IsSameSkelFunctor{ isSameSkelLam }, skeleotnAsset->GetSkeleton()->GetHash());

			if (mesh == nullptr)
				return false;

			const std::wstring sceneName = skeleotnAsset->GetName() + L"##_EditorScene";
			JDirectory* dir = JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());

			const J_OBJECT_FLAG flag = Core::AddSQValueEnum(OBJECT_FLAG_DO_NOT_SAVE, OBJECT_FLAG_UNEDITABLE);
			JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(sceneName,
				Core::MakeGuid(),
				flag,
				dir,
				J_SCENE_USE_CASE_TYPE::THREE_DIMENSIONAL_PREVIEW));
			JSceneManager::Instance().TryOpenScene(newScene, false);
			 
			JGameObject* meshObj = JGFI::Create(mesh->GetName(), Core::MakeGuid(), flag, *newScene->GetRootGameObject());
			JCFU::CreateRenderItem(Core::MakeGuid(), *meshObj, mesh);

			std::vector<std::vector<uint8>> skeletonVec = skeleotnAsset->GetSkeletonTreeIndexVec();
			std::vector<JGameObject*> skeletonTree((uint)skeletonVec.size());

			skeletonTree[0] = JGFI::Create(skeleotnAsset->GetJointName(0), Core::MakeGuid(), flag, *meshObj);
			const uint jointCount = (uint)skeletonVec.size();
			for (uint i = 0; i < jointCount; ++i)
			{
				const uint childrenCount = (uint)skeletonVec[i].size();
				for (uint j = 0; j < childrenCount; ++j)
				{
					const uint index = skeletonVec[i][j];
					skeletonTree[index] = JGFI::Create(skeleotnAsset->GetJointName(index),
						Core::MakeGuid(), 
						flag,
						*skeletonTree[i]);
				}
			}
			avatarScene = Core::GetUserPtr(newScene);
			explorer->Initialize(Core::GetUserPtr(meshObj));
			avatarEdit->Initialize(skeleotnAsset);
			avatarObserver->Initialize(avatarScene, L"Skeleton Edit Cam"); 
			return true;
		}
		void JEditorSkeletonPage::DoSetClose()noexcept
		{
			JEditorPage::DoSetClose();
			if (avatarScene.IsValid())
				JObject::BeginDestroy(avatarScene.Release());
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
			ImGui::Begin(GetName().c_str()); ImGui::End();
			ImGui::Begin(explorer->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarEdit->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarObserver->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarDetail->GetName().c_str()); ImGui::End();

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(GetDockNodeName().c_str());
		 
			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId);
			ImGui::DockBuilderSetNodePos(dockspaceId, viewport->Size);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockAvatarSceneEditor = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 1, &dock_main, &dock_main);
			ImGuiID dockSkeletonExplorer = ImGui::DockBuilderSplitNode(dockAvatarSceneEditor, ImGuiDir_Left, 0.2f, nullptr, &dockAvatarSceneEditor);
			ImGuiID dockAvatarEdit = ImGui::DockBuilderSplitNode(dockSkeletonExplorer, ImGuiDir_Down, 0.5f, nullptr, &dockSkeletonExplorer);
			ImGuiID dockAvatarDetail = ImGui::DockBuilderSplitNode(dockAvatarSceneEditor, ImGuiDir_Right, 0.2f, nullptr, &dockAvatarSceneEditor);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(explorer->GetName().c_str(), dockSkeletonExplorer);
			ImGui::DockBuilderDockWindow(avatarEdit->GetName().c_str(), dockAvatarEdit);
			ImGui::DockBuilderDockWindow(avatarObserver->GetName().c_str(), dockAvatarSceneEditor);
			ImGui::DockBuilderDockWindow(avatarDetail->GetName().c_str(), dockAvatarDetail);
			ImGui::DockBuilderFinish(dockspaceId);
		}
	}
}