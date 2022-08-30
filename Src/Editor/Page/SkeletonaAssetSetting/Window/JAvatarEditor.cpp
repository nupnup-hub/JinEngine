#include"JAvatarEditor.h" 
#include"../../JEditorAttribute.h"
#include"../../../Transition/JEditorTransition.h"
#include"../../../GuiLibEx/ImGuiEx/JImGuiImpl.h"  
#include"../../../../Object/GameObject/JGameObject.h"
#include"../../../../Object/Resource/Model/JModel.h"
#include"../../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../../Utility/JCommonUtility.h"
#include"../../../../../Lib/imgui/imgui.h"

namespace JinEngine
{
	namespace Editor
	{ 
		JAvatarEditor::JAvatarEditor(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType)
			:JEditorWindow(std::move(attribute),ownerPageType),
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
		void JAvatarEditor::UpdateWindow()
		{ 
			if (targetModel.IsValid())
			{ 
				JEditorWindow::UpdateWindow();
				BuildAvatarEdit(); 
			}		 
		}
		J_EDITOR_WINDOW_TYPE JAvatarEditor::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::AVATAR_EDITOR;
		}
		void JAvatarEditor::SetTargetModel(const Core::JUserPtr<JModel>& newTargetModel)noexcept
		{
			targetModel = newTargetModel;
		}
		void JAvatarEditor::BuildAvatarEdit()
		{ 
			JVector2<float> btnSize{ (float)JImGuiImpl::GetWindowSize().x, (float)JImGuiImpl::GetTextSize().y };
			if (JImGuiImpl::Button("Make JAvatar", btnSize))
			{
				auto doBinder = Core::JBindHandle(makeAvatarFunctor);
				auto undoBinder = Core::JBindHandle(clearAvatarFunctor);
				JEditorTransition::Execute(JEditorTask{ doBinder, "Make Avatar" }, JEditorTask{ undoBinder, "Cancel Make Avatar" });
			}
			if (hasAvatar)
			{
				JImGuiImpl::BeginTabBar("AvatarSetting");
				for (int i = 0; i < tabs.size(); ++i)
				{
					if (JImGuiImpl::TabItemButton(JAvatar::tabName[i].c_str()))
					{
						int doIndex = i; 
						auto doBinder = Core::JBindHandle(selectTabFunctor, std::move(doIndex));
						auto undoBinder = Core::JBindHandle(selectTabFunctor, std::move(FindSelectedTab()));
						JEditorTransition::Execute(JEditorTask{ doBinder, "Selecte Tab" }, JEditorTask{ undoBinder, "Cancel Selecte Tab" });
					}
				}
				JImGuiImpl::EndTabBar();
				int index = 0;
				for (index = 0; index < tabs.size(); ++index)
				{
					if (tabs[index])
						break;
				}

				int spaceOffset = 25;
				for (int i = 0; i < JAvatar::jointGuide[index].size(); ++i)
				{
					JImGuiImpl::Text(JAvatar::jointGuide[index][i].guideName);
					int textSize = (int)JAvatar::jointGuide[index][i].guideName.size();

					ImGui::SameLine();
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (spaceOffset - textSize) * JImGuiImpl::GetTextSize().x);

					int jointRefIndex = JAvatar::jointGuide[index][i].index;
					int nowRefValue = targetAvatar.jointReference[jointRefIndex];

					std::string nowRefJointName;
					if (nowRefValue == JSkeletonFixedData::incorrectJointIndex)
						nowRefJointName = "None##" + std::to_string(i);
					else
						nowRefJointName = JCUtil::WstrToU8Str(targetModel.Get()->GetSkeletonAsset()->GetSkeleton()->GetJointName(nowRefValue));

					if (!isValidJointRef[jointRefIndex])
						JImGuiImpl::SetColor(ImVec4(failColor.x, failColor.y, failColor.z, failColor.w), ImGuiCol_Button);

					if (JImGuiImpl::Button((nowRefJointName).c_str(), JVector2<float>(200, 20)))
					{
						int doJointRefIndex = jointRefIndex;
						int undoJointRefIndex = jointRefIndex;
						std::string doName;
						std::string undoName;

						//함수 실행시 isOpenJointSelector = !isOpenJointSelector이 되므로
						if (!isOpenJointSelector)
						{
							doName = "Open Joint Selector";
							undoName = "Cancel Open Joint Selector";
						}
						else
						{
							doName = "Cancel Open Joint Selector";
							undoName = "Open Joint Selector";
						}
						auto doBinder = Core::JBindHandle(openJointSelectorFunctor, std::move(doJointRefIndex));
						auto undoBinder = Core::JBindHandle(openJointSelectorFunctor, std::move(undoJointRefIndex));
						JEditorTransition::Execute(JEditorTask{ doBinder,  doName }, JEditorTask{ undoBinder, undoName });
					}

					ImGui::SameLine();
					if (JImGuiImpl::Button(("##" + nowRefJointName + "erase").c_str(), JVector2<float>(20, 20)))
					{
						int doJointRefIndex = jointRefIndex;
						int doJointIndex = JSkeletonFixedData::incorrectJointIndex;
						int undoJointRefIndex = jointRefIndex;
						int undoJointIndex = targetAvatar.jointReference[jointRefIndex];

						auto doBinder = Core::JBindHandle(setJointRefFunctor, std::move(doJointRefIndex), std::move(doJointIndex));
						auto undoBinder = Core::JBindHandle(setJointRefFunctor, std::move(undoJointRefIndex), std::move(undoJointIndex));
						JEditorTransition::Execute(JEditorTask{doBinder, "Erase Joint Reference" }, 
							JEditorTask{ undoBinder, "Cancel Erase Joint Reference"});
					}

					if (!isValidJointRef[jointRefIndex])
						JImGuiImpl::SetColorToDefault(ImGuiCol_Button);
				}

				if (JImGuiImpl::Button("Auto", JVector2<float>(75, 20)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					auto doBinder = Core::JBindHandle(setAllJointRefByAutoFunctor);
					auto undoBinder = Core::JBindHandle(setAllJointRefByVecFunctor, std::move(preJointRef));

					JEditorTransition::Execute(JEditorTask{ doBinder, "Auto Generate Joint Reference" },
						JEditorTask{ undoBinder, "Cancel Auto Generate Joint Reference" });
				}

				ImGui::SameLine();
				if (JImGuiImpl::Button("Save", JVector2<float>(75, 20)))
				{
					if (CheckAllJoint())
					{
						StoreAvatarData();
						JEditorTransition::Log("Store Avatar Data");
					}
				}

				ImGui::SameLine();
				if (JImGuiImpl::Button("Clear", JVector2<float>(75, 20)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					auto doBinder = Core::JBindHandle(clearJointRefFunctor);
					auto undoBinder = Core::JBindHandle(setAllJointRefByVecFunctor, std::move(preJointRef));
					   
					JEditorTransition::Execute(JEditorTask{ doBinder, "Clear Joint Reference" },
						JEditorTask{ undoBinder, "Cancel Clear Joint Reference" });
				}

				if (isOpenJointSelector)
				{
					JImGuiImpl::BeginWindow("Selector", &isOpenJointSelector); 
					BuildObjectExplorer(targetModel.Get()->ModelSceneInterface()->GetSkeletonRoot(), 0);
					JImGuiImpl::EndWindow();
				}
			}
		}
		void JAvatarEditor::BuildObjectExplorer(JGameObject* obj, uint index)
		{
			if ((obj->GetFlag() & OBJECT_FLAG_HIDDEN) > 0)
				return;

			std::string name = JCUtil::WstrToU8Str(obj->GetName());
			if (selectJointIndex == index)
				JImGuiImpl::SetColor(ImVec4(0.26f, 0.59f, 0.98f, 0.11f), ImGuiCol_Header);
			bool arrowClick = ArrowClick(name);

			bool res = JImGuiImpl::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow);
			if (selectJointIndex == index)
				JImGuiImpl::SetColorToDefault(ImGuiCol_Header);
			 
			if (ImGui::IsItemClicked() && !arrowClick)
			{
				if (selectJointIndex == index)
				{
					int doJointRefIndex = selectJointRefIndex;
					int doJointIndex = index;
					int undoJointRefIndex = selectJointRefIndex;
					int undoJointIndex = targetAvatar.jointReference[selectJointRefIndex];

					auto doBinder = Core::JBindHandle(setJointRefFunctor, std::move(doJointRefIndex), std::move(doJointIndex));
					auto undoBinder = Core::JBindHandle(setJointRefFunctor, std::move(undoJointRefIndex), std::move(undoJointIndex));
					JEditorTransition::Execute(JEditorTask{ doBinder, "Select Joint Reference" },
						JEditorTask{ undoBinder, "Cancel Select Joint Reference" });
					isOpenJointSelector = false;
				}
				else
					selectJointIndex = index;
			}
			if (res)
			{  
				const uint childrenCount = obj->GetChildrenCount();
				for (uint i = 0; i < childrenCount; ++i)
					BuildObjectExplorer(obj->GetChild(i), index + i);
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
			JSkeleton* skeleton = targetModel.Get()->GetSkeletonAsset()->GetSkeleton();
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
			JSkeletonAsset* skeletonAsset = targetModel.Get()->GetSkeletonAsset();
			uint32 maxJoint = (uint32)skeletonAsset->GetSkeleton()->GetJointCount();
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
				bool res = skeletonAsset->IsRegularChildJointIndex(targetAvatar.jointReference[i], parentSeletIndex);

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
		void JAvatarEditor::StoreAvatarData()
		{ 
			targetModel.Get()->GetSkeletonAsset()->AvatarInterface()->SetAvatar(&targetAvatar);
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

			if (targetModel.IsValid())
			{ 
				if (targetModel.Get()->GetSkeletonAsset()->HasAvatar())
				{
					hasAvatar = true;
					targetModel.Get()->GetSkeletonAsset()->AvatarInterface()->CopyAvatarJointIndex(&targetAvatar);
					CheckAllJoint();
				}
			}
		}
	}
}