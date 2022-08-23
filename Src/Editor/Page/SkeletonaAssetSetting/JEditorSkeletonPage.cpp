#include"JEditorSkeletonPage.h"
#include"Window/JAvatarEditor.h"
#include"../JEditorAttribute.h" 
#include"../CommonWindow/Detail/JObjectDetail.h"
#include"../CommonWindow/View/JEditorViewer.h"
#include"../CommonWindow/Explorer/JObjectExplorer.h"
#include"../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Object/Resource/JResourceManager.h" 
#include"../../../Object/Resource/Model/JModel.h"
#include"../../../Object/Resource/Skeleton/JSkeletonAsset.h"  
#include"../../../Core/Guid/GuidCreator.h"

namespace JinEngine
{
	namespace Editor
	{
		//const std::string& name, const size_t guid, const J_OBJECT_FLAG flag, std::unique_ptr<JEditorAttribute> attribute
		JEditorSkeletonPage::JEditorSkeletonPage(bool hasMetadata)
			:JEditorPage(std::make_unique<JEditorAttribute>("SkeletonAssetPage", Core::MakeGuid(), "SkeletonAssetPage",0.0f, 0.0f, 1.0f, 1.0f, false, false),
				true)
		{
			const uint memberWindowCount = 4;
			std::vector<std::string> nameVec
			{
				"SkeletonExplorer", "AvarEditor", "AvatarScene", "JAvatarDetail",
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
			};
			std::vector<bool> frontVec
			{
				true, true, true, true
			};
			std::vector<std::unique_ptr< JEditorAttribute>> windowAttributes;
			for (uint i = 0; i < memberWindowCount; ++i)
			{ 
				windowAttributes.push_back(std::make_unique<JEditorAttribute>(nameVec[i],
					initPosXRateVec[i],
					initPosYRateVec[i],
					initWidthRateVec[i],
					initHeightRateVec[i],
					openVec[i],
					frontVec[i]));
			} 

			/*
					std::unique_ptr<JAvatarEditor>avatarEdit;
			std::unique_ptr<JObjectDetail>avatarDetail;
			std::unique_ptr<JEditorViewer> modelViewer;
			std::unique_ptr<JObjectExplorer> skeletonExplorer;
			*/

			//수정필요
			skeletonExplorer = std::make_unique<JObjectExplorer>(std::move(windowAttributes[0]), GetGuid());
			avatarEdit = std::make_unique<JAvatarEditor>(std::move(windowAttributes[1]), GetGuid());
			modelViewer = std::make_unique<JEditorViewer>(std::move(windowAttributes[2]), GetGuid());
			avatarDetail = std::make_unique<JObjectDetail>(std::move(windowAttributes[3]), GetGuid());

			windows.resize(memberWindowCount);
			windows[0] = skeletonExplorer.get();
			windows[1] = avatarEdit.get();
			windows[2] = modelViewer.get();
			windows[3] = avatarDetail.get();

			if (!hasMetadata)
			{
				opendWindow.push_back(windows[0]);
				opendWindow.push_back(windows[1]);
				opendWindow.push_back(windows[2]);
				opendWindow.push_back(windows[3]);
			} 
		}
		JEditorSkeletonPage::~JEditorSkeletonPage() {}
		void JEditorSkeletonPage::Initialize(bool hasImguiTxt)
		{
			skeletonExplorer->Initialize();
			avatarEdit->Initialize();
			modelViewer->Initialize();
			avatarDetail->Initialize();

			if (!hasImguiTxt)
			{
				ImGuiWindowFlags groupFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

				ImGui::Begin(skeletonExplorer->GetName().c_str(), 0, groupFlag); ImGui::End();
				ImGui::Begin(avatarEdit->GetName().c_str(), 0, groupFlag); ImGui::End();
				ImGui::Begin(modelViewer->GetName().c_str(), 0, groupFlag); ImGui::End();
				ImGui::Begin(avatarDetail->GetName().c_str(), 0, groupFlag); ImGui::End();

				ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				window_flags |= ImGuiWindowFlags_NoBackground;

				ImGui::Begin(GetName().c_str(), 0, window_flags);
				ImGuiID dockspaceId = ImGui::GetID((dockSpaceName).c_str());

				ImGui::DockBuilderRemoveNode(dockspaceId);
				ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_None);
				ImGui::DockBuilderSetNodePos(dockspaceId, ImVec2(0, 0));

				ImGuiID dock_main = dockspaceId;
				ImGuiID dockSkeletonExplorer = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.2f, nullptr, &dock_main);
				ImGuiID dockAvatarEdit = ImGui::DockBuilderSplitNode(dockSkeletonExplorer, ImGuiDir_Down, 0.5f, nullptr, &dockSkeletonExplorer);
				ImGuiID dockAvatarSceneEditor = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.8f, nullptr, &dock_main);
				ImGuiID dockAvatarDetail = ImGui::DockBuilderSplitNode(dockAvatarSceneEditor, ImGuiDir_Right, 0.25f, nullptr, &dockAvatarSceneEditor);

				ImGui::DockBuilderDockWindow(GetName().c_str(), dock_main);
				ImGui::DockBuilderDockWindow(skeletonExplorer->GetName().c_str(), dockSkeletonExplorer);
				ImGui::DockBuilderDockWindow(avatarEdit->GetName().c_str(), dockAvatarEdit);
				ImGui::DockBuilderDockWindow(modelViewer->GetName().c_str(), dockAvatarSceneEditor);
				ImGui::DockBuilderDockWindow(avatarDetail->GetName().c_str(), dockAvatarDetail);
				ImGui::DockBuilderFinish(dockspaceId);
				ImGui::End();
				ImGui::SetNextWindowDockID();
			}
		}
		void JEditorSkeletonPage::UpdatePage()
		{
			ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
			ImGuiWindowFlags windowFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (JImGuiImpl::IsFullScreen())
				windowFlag |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
			else
				dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

			if (dockspaceFlag & ImGuiDockNodeFlags_PassthruCentralNode)
				windowFlag |= ImGuiWindowFlags_NoBackground;

			EnterPage(windowFlag, dockspaceFlag, false);
			uint8 opendWindowCount = (uint8)opendWindow.size();
			for (uint8 i = 0; i < opendWindowCount; ++i)
			{
				opendWindow[i]->EnterWindow();
				if (opendWindow[i]->IsActivated())
					opendWindow[i]->UpdateWindow();
				opendWindow[i]->CloseWindow();
			}
			ClosePage();
		}
		J_EDITOR_PAGE_TYPE JEditorSkeletonPage::GetPageType()const noexcept
		{
			return J_EDITOR_PAGE_TYPE::SKELETON_SETTING;
		}
		bool JEditorSkeletonPage::IsValidOpenRequest(JObject* selectedObj)noexcept
		{
			return IsValidOpenRequest(selectedObj);
		}
		bool JEditorSkeletonPage::StuffSkeletonAssetData(JObject* selectedObj)
		{
			if (selectedObj == nullptr || selectedObj->GetObjectType() != J_OBJECT_TYPE::RESOURCE_OBJECT)
				return false;

			JResourceObject* jRobj = static_cast<JResourceObject*>(selectedObj);

			if (jRobj->GetResourceType() != J_RESOURCE_TYPE::MODEL)
				return false;

			JModel* model = static_cast<JModel*>(jRobj);

			JGameObject* skeletonRoot = model->ModelSceneInterface()->GetSkeletonRoot();
			JSkeletonAsset* skeletonAsset = model->GetSkeletonAsset();

			if (skeletonRoot != nullptr && skeletonAsset != nullptr)
			{
				 /*
				skeletonExplorer->Initialize();
				avatarEdit->Initialize();
				modelViewer->Initialize();
				avatarDetail->Initialize();
				 */
				avatarEdit->SetModelGuid(model->GetGuid());
				return true;
			}
			else
				return false;
		}
		void JEditorSkeletonPage::StorePage(std::wofstream& stream)
		{
			JEditorPage::StorePage(stream); 
		}
		void JEditorSkeletonPage::LoadPage(std::wifstream& stream)
		{
			JEditorPage::LoadPage(stream);
			std::wstring guide;
			std::wstring wstrPath; 
		}
	}
}