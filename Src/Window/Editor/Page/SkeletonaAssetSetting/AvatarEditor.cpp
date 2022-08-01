#include"AvatarEditor.h"
#include"SkeletonAssetSettingPageShareData.h"
#include"../EditorAttribute.h"
#include"../../ImGuiEx/ImGuiManager.h" 
#include"../../Utility/EditorUtility.h"
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	AvatarEditor::AvatarEditor(std::unique_ptr<EditorAttribute> attribute,
		const size_t ownerPageGuid,
		SkeletonAssetSettingPageShareData* pageShareData)
		:EditorWindow(std::move(attribute), ownerPageGuid), pageShareData(pageShareData)
	{

	}
	void AvatarEditor::Initialize(EditorUtility* editorUtility)noexcept
	{
	}
	bool AvatarEditor::Activate(EditorUtility* editorUtility)
	{
		if (EditorWindow::Activate(editorUtility))
		{
			tabs[0] = true;
			tabs[1] = false;
			tabs[2] = false;
			tabs[3] = false;

			if (jointChecker.size() == 0)
				jointChecker.resize(JSkeletonFixedData::maxAvatarJointCount);

			ClearAllJointReference();
			if (pageShareData->skeletonAsset->HasAvatar())
			{
				hasAvatar = true;
				pageShareData->skeletonAsset->CopyAvatarJointIndex(&targetAvatar);
				CheckAllJoint();
			}
			return true;
		}
		else
			return false;
	}
	void AvatarEditor::UpdateWindow(EditorUtility* editorUtility)
	{
		EditorWindow::UpdateWindow(editorUtility);
		BuildAvatarEdit(editorUtility);
	}
	void AvatarEditor::BuildAvatarEdit(EditorUtility* editorUtility)
	{
		ImVec2 windowSize = ImGui::GetWindowSize();
		if (ImGui::Button("Make JAvatar", ImVec2(windowSize.x, editorUtility->textHeight)))
		{
			if (!hasAvatar)
			{
				StuffAllJointReferenceByAuto();
				CheckAllJoint();
			}
			hasAvatar = true;
		}
		if (hasAvatar)
		{
			ImGui::BeginTabBar("AvatarSetting");
			for (int i = 0; i < tabs.size(); ++i)
			{
				if (ImGui::TabItemButton(targetAvatar.tabName[i].c_str()))
				{
					CloseAllTab();
					tabs[i] = true;
				}
			}
			ImGui::EndTabBar();
			int index = 0;
			for (index = 0; index < tabs.size(); ++index)
			{
				if (tabs[index])
					break;
			}

			int spaceOffset = 25;
			for (int i = 0; i < targetAvatar.jointGuide[index].size(); ++i)
			{
				ImGui::Text(targetAvatar.jointGuide[index][i].guideName.c_str());
				int textSize = (int)targetAvatar.jointGuide[index][i].guideName.size();
				ImGui::SameLine();

				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (spaceOffset - textSize) * editorUtility->textWidth);

				int jointIndex = targetAvatar.jointGuide[index][i].index;
				int nowRefValue = targetAvatar.jointReference[jointIndex];
				std::string nowRefJointName;
				if (nowRefValue == JSkeletonFixedData::incorrectJointIndex)
					nowRefJointName = "None##" + std::to_string(i);
				else
					nowRefJointName = pageShareData->skeletonAsset->GetSkeleton()->GetJointName(nowRefValue);

				if (!jointChecker[jointIndex])
					ImGuiManager::SetColor(ImVec4(failColor.x, failColor.y, failColor.z, failColor.a), ImGuiCol_Button);

				if (ImGui::Button((nowRefJointName).c_str(), ImVec2(200, 20)))
				{
					isOpenJointSelector = !isOpenJointSelector;
					selectJointName = " ";
					selectedJointGroupNumber = i;
					selectJointRefNumber = jointIndex;
				}
				ImGui::SameLine();
				if (ImGui::Button(("##" + nowRefJointName + "erase").c_str(), ImVec2(20, 20)))
					targetAvatar.jointReference[jointIndex] = JSkeletonFixedData::incorrectJointIndex;

				if (!jointChecker[jointIndex])
					ImGuiManager::ReturnDefaultColor(ImGuiCol_Button);
			}
			if (ImGui::Button("Auto", ImVec2(75, 20)))
				StuffAllJointReferenceByAuto();
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(75, 20)))
			{
				if (CheckAllJoint())
					StoreAvatarData();
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear", ImVec2(75, 20)))
				ClearAllJointReference();
			if (isOpenJointSelector)
			{
				ImGui::Begin("Selector", &isOpenJointSelector);
				BuildObjectExplorer(editorUtility);
				ImGui::End();
			}
		}
	}
	void AvatarEditor::CloseAllTab()
	{
		for (int i = 0; i < tabs.size(); ++i)
			tabs[i] = false;
	}
	void AvatarEditor::BuildObjectExplorer(EditorUtility* editorUtility)
	{
		std::string name = pageShareData->skeletonRoot->GetName();
		if (selectJointName == name)
			ImGuiManager::SetColor(ImVec4(0.26f, 0.59f, 0.98f, 0.11f), ImGuiCol_Header);
		bool arrowClick = ArrowClick(name);
		bool res = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow);
		if (selectJointName == name)
			ImGuiManager::ReturnDefaultColor(ImGuiCol_Header);
		if (ImGui::IsItemClicked() && !arrowClick)
		{
			if (selectJointName == name)
			{
				StuffSelectedJointReference(pageShareData->skeletonRoot);
				CheckAllJoint();
			}
			else
				selectJointName = name;
		}
		if (res)
		{
			ObjectExplorerOnScreen(pageShareData->skeletonRoot, editorUtility);
			ImGui::TreePop();
		}
	}
	void AvatarEditor::ObjectExplorerOnScreen(JGameObject* obj, EditorUtility* editorUtility)
	{
		const uint childrenCount = obj->GetChildrenCount();
		for (uint i = 0; i < childrenCount; ++i)
		{
			JGameObject* child = obj->GetChild(i);
			if ((child->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
				continue;

			std::string name = child->GetName();
			if (selectJointName == name)
				ImGuiManager::SetColor(ImVec4(0.26f, 0.59f, 0.98f, 0.11f), ImGuiCol_Header);

			bool arrowClick = ArrowClick(name);
			bool res = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow);
			if (selectJointName == name)
				ImGuiManager::ReturnDefaultColor(ImGuiCol_Header);

			if (ImGui::IsItemClicked() && !arrowClick)
			{
				if (selectJointName == name)
				{
					StuffSelectedJointReference(child);
					CheckAllJoint();
				}
				else
					selectJointName = name;
			}
			if (res)
			{
				ObjectExplorerOnScreen(child, editorUtility);
				ImGui::TreePop();
			}
		}
	}
	void AvatarEditor::StuffSelectedJointReference(JGameObject* obj)
	{
		std::vector<uint> index;
		uint sameCount = 0;
		uint jointCount = (uint)pageShareData->skeletonAsset->GetSkeleton()->GetJointCount();
		size_t seletGuid = JCommonUtility::CalculateGuid(selectJointName);
		JSkeleton* skeleton = pageShareData->skeletonAsset->GetSkeleton();
		for (uint i = 0; i < jointCount; ++i)
		{
			const size_t targetGuid = JCommonUtility::CalculateGuid(skeleton->GetJointName(i));
			if (targetGuid == seletGuid)
			{
				index.push_back(i);
				++sameCount;
			}
		}
		if (sameCount == 1)
		{
			targetAvatar.jointReference[selectJointRefNumber] = (uint8)index[0];
			isOpenJointSelector = false;
			jointChecker[selectJointRefNumber] = true;
		}
		else if (sameCount > 1)
		{
			std::string objPath;
			JGameObject* nowParent = obj->GetParent();
			std::string reversePath = obj->GetName();
			while (nowParent != nullptr)
			{
				reversePath += "\\" + nowParent->GetName();
				nowParent = nowParent->GetParent();
			}
			size_t gameObjGuid = JCommonUtility::CalculateGuid(reversePath);

			JSkeleton* skeleton = pageShareData->skeletonAsset->GetSkeleton();
			for (int i = 0; i < index.size(); ++i)
			{
				reversePath.clear();
				int parent = (int)skeleton->GetJointParentIndex(index[i]);
				reversePath += skeleton->GetJointName(index[i]);

				while (parent != JSkeletonFixedData::incorrectJointIndex)
				{
					reversePath += skeleton->GetJointName(parent);
					parent = (int)skeleton->GetJointParentIndex(parent);
				}
				size_t skelGuid = JCommonUtility::CalculateGuid(reversePath);

				if (skelGuid == gameObjGuid)
				{
					targetAvatar.jointReference[selectJointRefNumber] = (uint8)index[i];
					jointChecker[selectJointRefNumber] = true;
					isOpenJointSelector = false;
					break;
				}
			}
		}
	}
	void AvatarEditor::StuffAllJointReferenceByAuto()
	{
		JSkeleton* skeleton = pageShareData->skeletonAsset->GetSkeleton();
		uint32 maxJoint = (uint32)skeleton->GetJointCount();
		uint32 partCount = (uint32)targetAvatar.jointGuide.size();

		for (uint32 i = 0; i < partCount; ++i)
		{
			uint32 jointCount = (uint32)targetAvatar.jointGuide[i].size();
			for (uint32 j = 0; j < jointCount; ++j)
			{
				for (uint32 k = 0; k < maxJoint; ++k)
				{
					if (targetAvatar.jointGuide[i][j].defaultJointName == skeleton->GetJointName(k))
					{
						int referenceIndex = targetAvatar.jointGuide[i][j].index;
						targetAvatar.jointReference[referenceIndex] = (uint8)k;
					}
				}
			}
		}
	}
	void AvatarEditor::StoreAvatarData()
	{
		pageShareData->skeletonAsset->SetAvatar(&targetAvatar);
	}
	void AvatarEditor::ClearAllJointReference()
	{
		uint32 referenceSize = (uint32)targetAvatar.jointReference.size();
		for (uint32 i = 0; i < referenceSize; ++i)
		{
			targetAvatar.jointReference[i] = JSkeletonFixedData::incorrectJointIndex;
			jointChecker[i] = true;
		}
	}
	bool AvatarEditor::CheckAllJoint()
	{
		JSkeleton* skeleton = pageShareData->skeletonAsset->GetSkeleton();
		uint32 maxJoint = (uint32)skeleton->GetJointCount();
		uint32 referenceSize = (uint32)targetAvatar.jointReference.size();

		for (uint32 i = 0; i < referenceSize; ++i)
		{
			if (targetAvatar.jointReference[i] == JSkeletonFixedData::incorrectJointIndex)
			{
				jointChecker[i] = true;
				continue;
			}
			uint8 parentSeletIndex = JSkeletonFixedData::incorrectJointIndex;
			uint8 parentIndex = targetAvatar.jointReferenceParent[i];
			while (parentIndex != JSkeletonFixedData::incorrectJointIndex)
			{
				if (targetAvatar.jointReference[parentIndex] != JSkeletonFixedData::incorrectJointIndex)
				{
					parentSeletIndex = targetAvatar.jointReference[parentIndex];
					break;
				}
				parentIndex = targetAvatar.jointReferenceParent[parentIndex];
			}
			if (parentSeletIndex == JSkeletonFixedData::incorrectJointIndex)
			{
				jointChecker[i] = true;
				continue;
			}
			bool res = pageShareData->skeletonAsset->IsRegularChildJointIndex(targetAvatar.jointReference[i], parentSeletIndex);

			if (res)
				jointChecker[i] = true;
			else
				jointChecker[i] = false;
		}
		return IsAllJointPassed();
	}
	bool AvatarEditor::IsAllJointPassed()noexcept
	{
		bool isPass = true;
		for (int i = 0; i < jointChecker.size(); ++i)
			isPass = jointChecker[i];
		return isPass;
	}
	bool AvatarEditor::ArrowClick(const std::string& name)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImGuiContext& g = *GImGui;

		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec2 padding = style.FramePadding;
		const ImVec2 label_size = ImGui::CalcTextSize(name.c_str(), NULL, false);

		const float text_offset_x = g.FontSize + padding.x * 3;
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;

		return (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
	}
}