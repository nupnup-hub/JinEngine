#include"JEditorSkeletonPage.h"
#include"Window/JAvatarEditor.h"
#include"../JEditorPageShareData.h"
#include"../JEditorAttribute.h"  
#include"../CommonWindow/View/JSceneViewer.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Object/GameObject/JGameObject.h"
#include"../../../Object/GameObject/JGameObjectFactory.h"
#include"../../../Object/Component/JComponentFactoryUtility.h"
#include"../../../Object/Resource/JResourceManager.h"  
#include"../../../Object/Resource/Mesh/JSkinnedMeshGeometry.h"
#include"../../../Object/Resource/Scene/JScene.h"
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
				"Avatar Viewer##SkeletonAssetPage",
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
			explorer = std::make_unique<JObjectExplorer>(windowNames[0], std::move(windowAttributes[0]), GetPageType());
			avatarEdit = std::make_unique<JAvatarEditor>(windowNames[1], std::move(windowAttributes[1]), GetPageType());
			avatarViewer = std::make_unique<JSceneViewer>(windowNames[2], std::move(windowAttributes[2]), GetPageType());
			avatarDetail = std::make_unique<JObjectDetail>(windowNames[2], std::move(windowAttributes[3]), GetPageType());

			windows.resize(memberWindowCount);
			windows[0] = explorer.get();
			windows[1] = avatarEdit.get();
			windows[2] = avatarViewer.get();
			windows[3] = avatarDetail.get();

			JEditorPageShareData::RegisterPage(GetPageType(), pageFlag);
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
			uint currOpWndCount = (uint)opendWindow.size();
			for (uint i = 0; i < currOpWndCount; ++i)
				CloseWindow(opendWindow[i]);

			OpenWindow(explorer.get());
			OpenWindow(avatarEdit.get());
			OpenWindow(avatarViewer.get());
			OpenWindow(avatarDetail.get()); 

			currOpWndCount = (uint)opendWindow.size();
			for (uint i = 0; i < currOpWndCount; ++i)
				opendWindow[i]->SetLastActivated(true); 
		}
		void JEditorSkeletonPage::Initialize()
		{

		}
		void JEditorSkeletonPage::UpdatePage()
		{ 
			/*
			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (JImGuiImpl::IsFullScreen())
				windowFlag |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
			else
				dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

			if (dockspaceFlag & ImGuiDockNodeFlags_PassthruCentralNode)
				windowFlag |= ImGuiWindowFlags_NoBackground;
			*/
			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground;

			if (!JImGuiImpl::IsFullScreen())
				dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

			EnterPage(windowFlag);
			if (reqInitDockNode)
			{
				BuildDockNode();
				reqInitDockNode = false;
			}
			UpdateDockSpace(dockspaceFlag);
			ClosePage();
			uint8 opendWindowCount = (uint8)opendWindow.size();
			for (uint8 i = 0; i < opendWindowCount; ++i)
				opendWindow[i]->UpdateWindow();
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
			if (!skeleotnAsset.ConnnectBaseUser(selectedObj))
				return false;

			auto isSameSkelLam = [](JSkinnedMeshGeometry* mesh, size_t skelHash)
			{
				JSkeletonAsset* skeletonAsset = mesh->GetSkeletonAsset();
				if (skeletonAsset != nullptr)
					return skeletonAsset->GetSkeleton()->GetHash() == skelHash;
				else
					return false;
			};
			using IsSameSkelFunctor = Core::JFunctor<bool, JSkinnedMeshGeometry*, size_t>;
			JMeshGeometry* mesh = JResourceManager::Instance().GetResourceByCondition<JSkinnedMeshGeometry, size_t>(IsSameSkelFunctor{ isSameSkelLam }, skeleotnAsset->GetSkeleton()->GetHash());

			if (mesh == nullptr)
				return false;

			const std::wstring sceneName = skeleotnAsset->GetName() + L"##_EditorScene";
			JDirectory* dir = JResourceManager::Instance().GetDirectory(JApplicationVariable::GetProjectDefaultResourcePath());

			const J_OBJECT_FLAG flag = Core::AddSQValueEnum(OBJECT_FLAG_UNIQUE_EDITOR_OBJECT, OBJECT_FLAG_UNEDITABLE);
			JScene* newScene = JRFI<JScene>::Create(Core::JPtrUtil::MakeOwnerPtr<JScene::InitData>(sceneName, Core::MakeGuid(), flag, dir));

			JGameObject* meshObj = JGFI::Create(*newScene->GetRootGameObject());
			JCFU::CreateRenderItem(Core::MakeGuid(), flag, *meshObj, mesh);

			std::vector<std::vector<uint8>> skeletonVec = skeleotnAsset->GetSkeletonTreeIndexVec();
			std::vector<JGameObject*> skeletonTree((uint)skeletonVec.size());

			skeletonTree[0] = JGFI::Create(skeleotnAsset->GetJointName(0), Core::MakeGuid(), flag, *meshObj);
			const uint jointCount = (uint)skeletonVec.size();
			for (uint i = 0; i < jointCount; ++i)
			{
				const uint childrenCount = (uint)skeletonVec[i].size();
				for (uint j = 0; j < childrenCount; ++j)
					skeletonTree[j] = JGFI::Create(skeleotnAsset->GetJointName(j), Core::MakeGuid(), OBJECT_FLAG_UNIQUE_EDITOR_OBJECT, *skeletonTree[i]);
			}
			avatarScene = Core::GetUserPtr(newScene);

			avatarEdit->Initialize(skeleotnAsset);
			avatarViewer->Initialize(avatarScene);

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
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGuiID dockspaceId = ImGui::GetID(GetDockNodeName().c_str());
		 
			ImGui::DockBuilderRemoveNode(dockspaceId);
			ImGui::DockBuilderAddNode(dockspaceId);
			ImGui::DockBuilderSetNodePos(dockspaceId, viewport->Size);

			ImGuiID dock_main = dockspaceId;
			ImGuiID dockAvatarSceneEditor = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Up, 1, &dock_main, &dock_main);
			ImGuiID dockAvatarDetail = ImGui::DockBuilderSplitNode(dockAvatarSceneEditor, ImGuiDir_Right, 0.3f, nullptr, &dockAvatarSceneEditor);
			ImGuiID dockSkeletonExplorer = ImGui::DockBuilderSplitNode(dockAvatarSceneEditor, ImGuiDir_Left, 0.3f, nullptr, &dockAvatarSceneEditor);
			ImGuiID dockAvatarEdit = ImGui::DockBuilderSplitNode(dockSkeletonExplorer, ImGuiDir_Down, 0.5f, nullptr, &dockSkeletonExplorer);

			ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
			ImGui::DockBuilderDockWindow(explorer->GetName().c_str(), dockSkeletonExplorer);
			ImGui::DockBuilderDockWindow(avatarEdit->GetName().c_str(), dockAvatarEdit);
			ImGui::DockBuilderDockWindow(avatarViewer->GetName().c_str(), dockAvatarSceneEditor);
			ImGui::DockBuilderDockWindow(avatarDetail->GetName().c_str(), dockAvatarDetail);
			ImGui::DockBuilderFinish(dockspaceId);

			ImGui::Begin(GetName().c_str()); ImGui::End();
			ImGui::Begin(explorer->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarEdit->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarViewer->GetName().c_str()); ImGui::End();
			ImGui::Begin(avatarDetail->GetName().c_str()); ImGui::End();
		}
	}
}