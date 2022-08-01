#include"EditorSkeletonAssetPage.h"
#include"AvatarEditor.h" 
#include"AvatarDetail.h" 
#include"SkeletonAssetSettingPageShareData.h" 
#include"../EditorAttribute.h"
#include"../Common/PreviewSceneEditor.h"
#include"../Common/SpecificObjectExplorer.h"  
#include"../../Utility/EditorUtility.h"
#include"../../ImGuiEx/ImGuiManager.h"  
 
#include"../../../../Object/Resource/JResourceManager.h" 
 
#include"../../../../Object/Resource/Model/JModel.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Scene/JScene.h"
#include"../../../../Object/Resource/Scene/Preview/PreviewScene.h"
#include"../../../../Object/Resource/Scene/Preview/PreviewSceneGroup.h"
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	EditorSkeletonAssetPage::EditorSkeletonAssetPage()
		:EditorWindowPage(std::make_unique<EditorAttribute>("SkeletonAssetPage",
			JCommonUtility::CalculateGuid("SkeletonAssetPage"),
			0.0f, 0.0f, 1.0f, 1.0f, false, false))
	{
		const uint memberWindowCount = 4;
		std::vector<std::string> nameVec
		{
			"SkeletonExplorer", "AvarEditor", "AvatarScene", "AvatarDetail",
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
		std::vector<std::unique_ptr< EditorAttribute>> windowAttributes;
		for (uint i = 0; i < memberWindowCount; ++i)
		{
			size_t guid = JCommonUtility::CalculateGuid(nameVec[i]);
			windowAttributes.push_back(std::make_unique<EditorAttribute>(nameVec[i],
				guid,
				initPosXRateVec[i],
				initPosYRateVec[i],
				initWidthRateVec[i],
				initHeightRateVec[i],
				openVec[i],
				frontVec[i]));
		}
		pageShareData = std::make_unique<SkeletonAssetSettingPageShareData>();

		skeletonExplorer = std::make_unique<SpecificObjectExplorer>(std::move(windowAttributes[0]), GetGuid(), pageShareData.get());
		avatarEdit = std::make_unique<AvatarEditor>(std::move(windowAttributes[1]), GetGuid(), pageShareData.get());
		avatarSceneEditor = std::make_unique<PreviewSceneEditor>(std::move(windowAttributes[2]), GetGuid());
		avatarDetail = std::make_unique<AvatarDetail>(std::move(windowAttributes[3]), GetGuid());

		windows.resize(memberWindowCount);
		windows[0] = skeletonExplorer.get();
		windows[1] = avatarEdit.get();
		windows[2] = avatarSceneEditor.get();
		windows[3] = avatarDetail.get();

		opendWindow.push_back(windows[0]);
		opendWindow.push_back(windows[1]);
		opendWindow.push_back(windows[2]);
		opendWindow.push_back(windows[3]);

		previewGroupGuid = JCommonUtility::CalculateGuid(GetName());
		previewGroup = JResourceManager::Instance().CreatePreviewGroup(GetName(), 1);
	}
	EditorSkeletonAssetPage::~EditorSkeletonAssetPage() {}
	void EditorSkeletonAssetPage::Initialize(EditorUtility* editorUtility, std::vector<EditorWindow*>& allEditorWindows, bool hasImguiTxt)
	{
		allEditorWindows.push_back(skeletonExplorer.get());
		allEditorWindows.push_back(avatarEdit.get());
		allEditorWindows.push_back(avatarSceneEditor.get());
		allEditorWindows.push_back(avatarDetail.get());

		skeletonExplorer->Initialize(editorUtility);
		avatarEdit->Initialize(editorUtility);
		avatarDetail->Initialize(editorUtility);

		if (!hasImguiTxt)
		{
			ImGuiWindowFlags groupFlag = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse;

			ImGui::Begin(skeletonExplorer->GetName().c_str(), 0, groupFlag); ImGui::End();
			ImGui::Begin(avatarEdit->GetName().c_str(), 0, groupFlag); ImGui::End();
			ImGui::Begin(avatarSceneEditor->GetName().c_str(), 0, groupFlag); ImGui::End();
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
			ImGui::DockBuilderDockWindow(avatarSceneEditor->GetName().c_str(), dockAvatarSceneEditor);
			ImGui::DockBuilderDockWindow(avatarDetail->GetName().c_str(), dockAvatarDetail);
			ImGui::DockBuilderFinish(dockspaceId);
			ImGui::End();
		}
	}
	bool EditorSkeletonAssetPage::Activate(EditorUtility* editorUtility)
	{
		if (!IsActivated() && IsOpen() && IsFront())
		{
			if (editorUtility->selectedObject == nullptr)
				return false;

			skeletonPath = dynamic_cast<JResourceObject*>(editorUtility->selectedObject)->GetPath();
			if (StuffSkeletonAssetData())
			{
				PreviewScene* previewScene = JResourceManager::Instance().CreatePreviewScene(previewGroup,
					pageShareData->model,
					PREVIEW_DIMENSION::THREE_DIMENTIONAL_RESOURCE,
					(PREVIEW_FLAG)((int)PREVIEW_FLAG::HAS_SKELETON | (int)PREVIEW_FLAG::NON_FIXED));
				avatarSceneEditor->Initialize(previewScene, 0);
				return EditorWindowPage::Activate(editorUtility);
			}
			else
				return false;
		}
		else
			return false;
	}
	bool EditorSkeletonAssetPage::DeActivate(EditorUtility* editorUtility)
	{
		if (EditorWindowPage::DeActivate(editorUtility))
		{
			JResourceManager::Instance().ClearPreviewGroup(previewGroup);
			return true;
		}
		else
			return false;
	}
	void EditorSkeletonAssetPage::StorePage(std::wofstream& stream)
	{
		EditorWindowPage::StorePage(stream);
		if (skeletonPath.empty())
			stream << L"SkeletonPath: " << L"Null";
		else
			stream << L"SkeletonPath: " << JCommonUtility::U8StringToWstring(skeletonPath);
	}
	void EditorSkeletonAssetPage::LoadPage(std::wifstream& stream, std::vector<EditorWindow*>& allEditorWindows, std::vector<EditorWindowPage*>& opendEditorPage, EditorUtility* editorUtility)
	{
		EditorWindowPage::LoadPage(stream, allEditorWindows, opendEditorPage, editorUtility);
		std::wstring guide;
		std::wstring wstrPath;

		stream >> guide; stream >> wstrPath;
		skeletonPath = JCommonUtility::WstringToU8String(wstrPath);
	}
	void EditorSkeletonAssetPage::UpdatePage(EditorUtility* editorUtility)
	{
		ImGuiDockNodeFlags dockspaceFlag = ImGuiDockNodeFlags_None;
		ImGuiWindowFlags windowFlag = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (ImGuiManager::IsFullScreen())
			windowFlag |= ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
		else
			dockspaceFlag &= ~ImGuiDockNodeFlags_PassthruCentralNode;

		if (dockspaceFlag & ImGuiDockNodeFlags_PassthruCentralNode)
			windowFlag |= ImGuiWindowFlags_NoBackground;

		EnterPage(windowFlag, dockspaceFlag, false);
		uint8 opendWindowCount = (uint8)opendWindow.size();
		for (uint8 i = 0; i < opendWindowCount; ++i)
		{
			opendWindow[i]->EnterWindow(editorUtility);
			if (opendWindow[i]->IsActivated())
				opendWindow[i]->UpdateWindow(editorUtility);
			opendWindow[i]->CloseWindow();
		}
		ClosePage(editorUtility);
	}
	bool EditorSkeletonAssetPage::StuffSkeletonAssetData()
	{
		std::string folderPath;
		std::string folderName;
		std::string format;
		JCommonUtility::DecomposeFilePath(skeletonPath, folderPath, folderName, format);
		std::string modelPath = folderPath + folderName + ".fbx";
		JModel* model = JResourceManager::Instance().GetResource<JModel>(JCommonUtility::CalculateGuid(modelPath));

		//수정필요
		if (model == nullptr)
			return false;

		JGameObject* skeletonRoot = model->GetSkeletonRoot();
		JSkeletonAsset* skeletonAsset = model->GetSkeletonAsset();

		if (skeletonRoot != nullptr && skeletonAsset != nullptr)
		{
			pageShareData->model = model;
			pageShareData->skeletonAsset = skeletonAsset;
			pageShareData->skeletonRoot = skeletonRoot;
			return true;
		}
		else
			return false;
	}
}
