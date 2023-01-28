#include"JAvatarEditor.h" 
#include"../../JEditorAttribute.h" 
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../Utility/JEditorLineCalculator.h"
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../../Core/Undo/JTransition.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	namespace Editor
	{
		JAvatarEditor::JAvatarEditor(const std::string& name,std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(name, std::move(attribute), ownerPageType),
			makeAvatarFunctor(&JAvatarEditor::MakeAvatar, this),
			clearAvatarFunctor(&JAvatarEditor::ClearAvatar, this),
			selectTabFunctor(&JAvatarEditor::SelectTab, this),
			openJointSelectorFunctor(&JAvatarEditor::OpenJointSelector, this),
			setJointRefFunctor(&JAvatarEditor::SetJointReference, this),
			setAllJointRefByVecFunctor(&JAvatarEditor::SetAllJointReferenceByVec, this),
			setAllJointRefByAutoFunctor(&JAvatarEditor::SetAllJointReferenceByAuto, this),
			clearJointRefFunctor(&JAvatarEditor::ClearJointReference, this)
		{ }
		JAvatarEditor::~JAvatarEditor()
		{ }
		J_EDITOR_WINDOW_TYPE JAvatarEditor::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::AVATAR_EDITOR;
		}
		void JAvatarEditor::Initialize(const Core::JUserPtr<JSkeletonAsset>& newTargetSkeleton)noexcept
		{
			targetSkeleton = newTargetSkeleton;
		}
		void JAvatarEditor::UpdateWindow()
		{
			EnterWindow(ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
			UpdateDocking();
			if (IsActivated() && targetSkeleton.IsValid())
			{
				UpdateMouseClick();
				preBtnColor = JImGuiImpl::GetColor(ImGuiCol_Button);
				BuildAvatarEdit();
			}
			CloseWindow();
		}
		void JAvatarEditor::BuildAvatarEdit()
		{ 		 
			const JVector2<float> wndSize = ImGui::GetWindowSize();
			const float xPaddingRate = 0.025f;
			const float xPadding = wndSize.x * xPaddingRate;

			const float MakeBtnWidth = wndSize.x * 0.25f;
			const float MakeBtnPos = xPadding + wndSize.x * 0.5f - MakeBtnWidth * 0.5f;
			ImGui::SetCursorPosX(MakeBtnPos);
			if (JImGuiImpl::Button("Make", JVector2<float>(MakeBtnWidth, 0)))
			{  
				Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Make Avatar",
					std::make_unique<MakeAvatarF::CompletelyBind>(makeAvatarFunctor),
					std::make_unique<ClearAvatarF::CompletelyBind>(clearAvatarFunctor)));
			} 
			if (hasAvatar)
			{
				JImGuiImpl::BeginTabBar("AvatarSetting");
				for (int i = 0; i < tabs.size(); ++i)
				{
					if (JImGuiImpl::TabItemButton(JAvatar::tabName[i].c_str()))
					{
						int doIndex = i; 
						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Selecte Tab",
							std::make_unique<SelectTabF::CompletelyBind>(selectTabFunctor, std::move(doIndex)),
							std::make_unique<SelectTabF::CompletelyBind>(selectTabFunctor, std::move(FindSelectedTab()))));
					}
				}
				JImGuiImpl::EndTabBar();

				int index = 0;
				for (index = 0; index < tabs.size(); ++index)
				{
					if (tabs[index])
						break;
				}
 
				//name explorer button  
				//const float btnHeight = JImGuiImpl::GetAlphabetSize().y + ImGui::GetStyle().FramePadding.y * 5.0f;
				const float btnHeight = ImGui::GetFrameHeight() * 1.2f;
				const JVector2<float> listSize = wndSize - JVector2<float>(xPadding * 2, btnHeight + ImGui::GetCursorPosY());
				
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
				ImGui::BeginListBox(("##AvatarEditor" + GetName()).c_str(), listSize);
				const float rate[3] = {listSize.x * 0.4f, listSize.x * 0.4f, listSize.x * 0.05f };
				JEditorStaticLineCalculator<3> lineCal(rate);
				lineCal.LabelOnScreen("Name");
				lineCal.LabelOnScreen("Part");
				lineCal.LabelOnScreen("Erase");
				ImGui::Separator();

				for (int i = 0; i < JAvatar::jointGuide[index].size(); ++i)
				{
					lineCal.SetNextContentsPosition();
					JImGuiImpl::Text(JAvatar::jointGuide[index][i].guideName); 
 
					int jointRefIndex = JAvatar::jointGuide[index][i].index;
					int nowRefValue = targetAvatar.jointReference[jointRefIndex];

					std::string nowRefJointName;
					if (nowRefValue == JSkeletonFixedData::incorrectJointIndex)
						nowRefJointName = "None##" + std::to_string(i);
					else
						nowRefJointName = JCUtil::WstrToU8Str(targetSkeleton->GetSkeleton()->GetJointName(nowRefValue));

					if (!isValidJointRef[jointRefIndex])
						JImGuiImpl::SetColor(ImVec4(failColor.x, failColor.y, failColor.z, failColor.w), ImGuiCol_Button);

					lineCal.SetNextContentsPosition();
					if (JImGuiImpl::Button((nowRefJointName).c_str(), JVector2<float>(rate[1], 0)))
					{
						int doJointRefIndex = jointRefIndex;
						int undoJointRefIndex = jointRefIndex;
						std::string taskName;

						//함수 실행시 isOpenJointSelector = !isOpenJointSelector이 되므로
						if (!isOpenJointSelector)
							taskName = "Open Joint Selector";
						else
							taskName = "Cancel Open Joint Selector";
						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName,
							std::make_unique<OpenJointSelectorF::CompletelyBind>(openJointSelectorFunctor, std::move(doJointRefIndex)),
							std::make_unique<OpenJointSelectorF::CompletelyBind>(openJointSelectorFunctor, std::move(undoJointRefIndex))));
					}
					 
					lineCal.SetNextContentsPosition();
					if (JImGuiImpl::Button(("##" + nowRefJointName + "erase").c_str(), JVector2<float>(rate[2], 0)))
					{
						int doJointRefIndex = jointRefIndex;
						int doJointIndex = JSkeletonFixedData::incorrectJointIndex;
						int undoJointRefIndex = jointRefIndex;
						int undoJointIndex = targetAvatar.jointReference[jointRefIndex];

						Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Erase Joint Reference",
							std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(doJointRefIndex), std::move(doJointIndex)),
							std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(undoJointRefIndex), std::move(undoJointIndex))));
					}

					if (!isValidJointRef[jointRefIndex])
						JImGuiImpl::SetColor(preBtnColor, ImGuiCol_Button);
				}
				ImGui::EndListBox();
				 
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xPadding);
				if (JImGuiImpl::Button("Auto", JVector2<float>(ImGui::CalcTextSize("_Auto_").x, 0)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Auto Generate Joint Reference",
						std::make_unique<SetAllJointRefByAutoF::CompletelyBind>(setAllJointRefByAutoFunctor),
						std::make_unique<SetAllJointRefByVecF::CompletelyBind>(setAllJointRefByVecFunctor, std::move(preJointRef))));
				}

				const float btnPaddingRate = 0.0025f;
				const float btnPadding = wndSize.x * btnPaddingRate;

				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPadding);
				if (JImGuiImpl::Button("Save", JVector2<float>(ImGui::CalcTextSize("_Save_").x, 0)))
				{
					if (CheckAllJoint())
					{
						StoreAvatarData();
						Core::JTransition::Log("Store Avatar Data");
					}
				}

				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnPadding);
				if (JImGuiImpl::Button("Clear", JVector2<float>(ImGui::CalcTextSize("_Clear_").x, 0)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Clear Joint Reference",
						std::make_unique<ClearJointRefF::CompletelyBind>(clearJointRefFunctor),
						std::make_unique<SetAllJointRefByVecF::CompletelyBind>(setAllJointRefByVecFunctor, std::move(preJointRef))));
				}

				if (isOpenJointSelector)
				{
					JImGuiImpl::BeginWindow("Selector", &isOpenJointSelector);
					BuildObjectExplorer(0, targetSkeleton->GetSkeletonTreeIndexVec());
					JImGuiImpl::EndWindow();
				}
			}
		}
		void JAvatarEditor::BuildObjectExplorer(const uint8 index, const std::vector<std::vector<uint8>>& treeIndexVec)
		{
			bool isSelected = selectJointIndex == index;
			if (isSelected)
				SetTreeNodeColor(JImGuiImpl::GetTreeDeepFactor());

			std::string name = JCUtil::WstrToU8Str(targetSkeleton->GetJointName(index));
			bool arrowClick = ArrowClick(name);

			bool res = JImGuiImpl::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow);
			if (isSelected)
				SetTreeNodeColor(-JImGuiImpl::GetTreeDeepFactor());

			if (ImGui::IsItemClicked() && !arrowClick)
			{
				if (isSelected)
				{
					int doJointRefIndex = selectJointRefIndex;
					int doJointIndex = index;
					int undoJointRefIndex = selectJointRefIndex;
					int undoJointIndex = targetAvatar.jointReference[selectJointRefIndex];

					Core::JTransition::Execute(std::make_unique<Core::JTransitionSetValueTask>("Select Joint Reference",
						std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(doJointRefIndex), std::move(doJointIndex)),
						std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(undoJointRefIndex), std::move(undoJointIndex))));
					isOpenJointSelector = false;
				}
				else
					selectJointIndex = index;
			}
			if (res)
			{
				const uint childrenCount = (uint)treeIndexVec[index].size();
				for (uint i = 0; i < childrenCount; ++i)
					BuildObjectExplorer(treeIndexVec[index][i], treeIndexVec);
				JImGuiImpl::TreePop();
			}
		}
		void JAvatarEditor::MakeAvatar()
		{
			if (!hasAvatar)
				SetAllJointReferenceByAuto();
			hasAvatar = true;
		}
		void JAvatarEditor::ClearAvatar()
		{
			ClearJointReference();
			hasAvatar = false;
		}
		void JAvatarEditor::SelectTab(const uint index)
		{
			CloseAllTab();
			if (index < tabs.size())
				tabs[index] = true;
		}
		void JAvatarEditor::OpenJointSelector(const int jointRefIndx)
		{
			isOpenJointSelector = !isOpenJointSelector;
			selectJointIndex = 0;
			selectJointRefIndex = jointRefIndx;
		}
		void JAvatarEditor::SetJointReference(const int jointRefIndex, const int jointIndex)
		{
			targetAvatar.jointReference[jointRefIndex] = jointIndex;
			CheckAllJoint();
		}
		void JAvatarEditor::SetAllJointReferenceByVec(std::vector<uint8>&& vec)
		{
			for (uint i = 0; i < JSkeletonFixedData::maxAvatarJointCount; ++i)
				targetAvatar.jointReference[i] = std::move(vec[i]);
			CheckAllJoint();
		}
		void JAvatarEditor::SetAllJointReferenceByAuto()
		{
			JSkeleton* skeleton = targetSkeleton->GetSkeleton();
			uint32 maxJoint = (uint32)skeleton->GetJointCount();
			uint32 partCount = (uint32)JAvatar::jointGuide.size();

			for (uint32 i = 0; i < partCount; ++i)
			{
				uint32 jointCount = (uint32)JAvatar::jointGuide[i].size();
				for (uint32 j = 0; j < jointCount; ++j)
				{
					for (uint32 k = 0; k < maxJoint; ++k)
					{
						if (JCUtil::StrToWstr(JAvatar::jointGuide[i][j].defaultJointName) == skeleton->GetJointName(k))
						{
							int referenceIndex = JAvatar::jointGuide[i][j].index;
							targetAvatar.jointReference[referenceIndex] = (uint8)k;
						}
					}
				}
			}
			CheckAllJoint();
		}
		void JAvatarEditor::ClearJointReference()
		{
			uint32 referenceSize = (uint32)targetAvatar.jointReference.size();
			for (uint32 i = 0; i < referenceSize; ++i)
			{
				targetAvatar.jointReference[i] = JSkeletonFixedData::incorrectJointIndex;
				isValidJointRef[i] = true;
			}
		}
		void JAvatarEditor::CloseAllTab() noexcept
		{
			for (int i = 0; i < tabs.size(); ++i)
				tabs[i] = false;
		}
		int JAvatarEditor::FindSelectedTab()noexcept
		{
			for (int i = 0; i < tabs.size(); ++i)
			{
				if (tabs[i])
					return i;
			}
			return -1;
		}
		bool JAvatarEditor::CheckAllJoint()noexcept
		{
			uint32 maxJoint = (uint32)targetSkeleton->GetSkeleton()->GetJointCount();
			uint32 referenceSize = (uint32)targetAvatar.jointReference.size();

			for (uint32 i = 0; i < referenceSize; ++i)
			{
				if (targetAvatar.jointReference[i] == JSkeletonFixedData::incorrectJointIndex)
				{
					isValidJointRef[i] = true;
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
					parentIndex = JAvatar::jointReferenceParent[parentIndex];
				}
				if (parentSeletIndex == JSkeletonFixedData::incorrectJointIndex)
				{
					isValidJointRef[i] = true;
					continue;
				}
				bool res = targetSkeleton->IsRegularChildJointIndex(targetAvatar.jointReference[i], parentSeletIndex);

				if (res)
					isValidJointRef[i] = true;
				else
					isValidJointRef[i] = false;
			}
			return IsAllJointPassed();
		}
		bool JAvatarEditor::IsAllJointPassed()noexcept
		{
			bool isPass = true;
			for (int i = 0; i < isValidJointRef.size(); ++i)
				isPass = isPass && isValidJointRef[i];
			return isPass;
		}
		bool JAvatarEditor::ArrowClick(const std::string& name)
		{
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			ImGuiContext& g = *GImGui;

			ImGuiStyle& style = ImGui::GetStyle();
			const JVector2<float> padding = style.FramePadding;
			const JVector2<float> label_size = ImGui::CalcTextSize(name.c_str(), NULL, false);

			const float text_offset_x = g.FontSize + padding.x * 3;
			const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);
			const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
			JVector2<float> text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
			const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
			const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;

			return (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
		}
		void JAvatarEditor::StoreAvatarData()
		{
			targetSkeleton->AvatarInterface()->SetAvatar(&targetAvatar);
			SetModifiedBit(targetSkeleton, true);
		}
		void JAvatarEditor::DoActivate() noexcept
		{
			JEditorWindow::DoActivate();

			isOpenAvatarSetting = false;
			isOpenJointSelector = false;
			hasAvatar = false;

			selectJointRefIndex = -1;
			selectJointIndex = -1;

			tabs[0] = true;
			tabs[1] = false;
			tabs[2] = false;
			tabs[3] = false;

			ClearJointReference();

			if (targetSkeleton.IsValid())
			{
				if (targetSkeleton->HasAvatar())
				{
					hasAvatar = true;
					targetSkeleton->AvatarInterface()->CopyAvatarJointIndex(&targetAvatar);
					CheckAllJoint();
				}
			}
		}
	}
}