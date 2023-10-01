#include"JAvatarEditor.h" 
#include"../../JEditorAttribute.h" 
#include"../../../Gui/JGui.h"  
#include"../../../Align/JEditorAlignCalculator.h"
#include"../../../../Object/GameObject/JGameObject.h" 
#include"../../../../Object/Resource/Skeleton/JSkeleton.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAsset.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonAssetPrivate.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h" 
#include"../../../../Core/Transition/JTransition.h"
#include"../../../../Core/Utility/JCommonUtility.h" 
#include"../../../../Core/Math/JVectorExtend.h" 

namespace JinEngine
{
	namespace Editor
	{
		namespace
		{
			using AvatarInterface = JSkeletonAssetPrivate::AvatarInterface;
		}
		JAvatarEditor::JAvatarEditor(const std::string& name, 
			std::unique_ptr<JEditorAttribute> attribute, 
			const J_EDITOR_PAGE_TYPE ownerPageType,
			const J_EDITOR_WINDOW_FLAG windowFlag)
			:JEditorWindow(name, std::move(attribute), ownerPageType, windowFlag),
			makeAvatarFunctor(&JAvatarEditor::MakeAvatar, this),
			clearAvatarFunctor(&JAvatarEditor::ClearAvatar, this),
			selectTabFunctor(&JAvatarEditor::SelectTab, this),
			openJointSelectorFunctor(&JAvatarEditor::OpenJointSelector, this),
			setJointRefFunctor(&JAvatarEditor::SetJointReference, this),
			setAllJointRefByVecFunctor(&JAvatarEditor::SetAllJointReferenceByVec, this),
			setAllJointRefByAutoFunctor(&JAvatarEditor::SetAllJointReferenceByAuto, this),
			clearJointRefFunctor(&JAvatarEditor::ClearJointReference, this)
		{  }
		JAvatarEditor::~JAvatarEditor()
		{ }
		J_EDITOR_WINDOW_TYPE JAvatarEditor::GetWindowType()const noexcept
		{
			return J_EDITOR_WINDOW_TYPE::AVATAR_EDITOR;
		}
		void JAvatarEditor::Initialize(const JUserPtr<JSkeletonAsset>& newTargetSkeleton)noexcept
		{
			targetSkeleton = newTargetSkeleton;
		}
		void JAvatarEditor::UpdateWindow()
		{
			EnterWindow(J_GUI_WINDOW_FLAG_NO_SCROLL_BAR | J_GUI_WINDOW_FLAG_NO_COLLAPSE);
			UpdateDocking();
			if (IsActivated() && targetSkeleton.IsValid())
			{
				UpdateMouseClick(); 
				BuildAvatarEdit();
			}
			CloseWindow();
		}
		void JAvatarEditor::BuildAvatarEdit()
		{ 
			const JVector2<float> wndSize = JGui::GetWindowSize();
			constexpr float xPaddingRate = 0.025f;
			const float xPadding = wndSize.x * xPaddingRate;
			const float frameBorderSize = JGui::GetFrameBorderSize();

			const float MakeBtnWidth = wndSize.x * 0.25f;
			const float MakeBtnPos = xPadding + wndSize.x * 0.5f - MakeBtnWidth * 0.5f;
			JGui::SetCursorPosX(MakeBtnPos);
			if (JGui::Button("Make", JVector2<float>(MakeBtnWidth, 0)))
			{
				JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Make avatar",
					"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
					std::make_unique<MakeAvatarF::CompletelyBind>(makeAvatarFunctor),
					std::make_unique<ClearAvatarF::CompletelyBind>(clearAvatarFunctor)));
			}
			if (hasAvatar)
			{
				JGui::SetCursorPosX(JGui::GetCursorPosX() + xPadding);
				if(JGui::BeginTabBar("AvatarSetting"))
				{
					for (int i = 0; i < tabs.size(); ++i)
					{
						if (JGui::BeginTabItem(JAvatar::tabName[i]))
						{
							int doIndex = i;
							JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Select avatar part",
								"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
								std::make_unique<SelectTabF::CompletelyBind>(selectTabFunctor, std::move(doIndex)),
								std::make_unique<SelectTabF::CompletelyBind>(selectTabFunctor, std::move(FindSelectedTab()))));
							JGui::EndTabItem();
						}
					}
					JGui::EndTabBar();
				}
				//JGui::SetCursorPosX(JGui::GetCursorPosX() + xPadding);

				int index = 0;
				for (index = 0; index < tabs.size(); ++index)
				{
					if (tabs[index])
						break;
				}

				//name explorer button  
				//const float btnHeight = JGui::GetAlphabetSize().y + JGui::GetStyle().FramePadding.y * 5.0f;
				/*
				 ImGuiContext& g = *GImGui;
				return (g.FontSize + g.Style.FramePadding.y * 2.0f) * 1.2f
				
				//ImGui::CalcItemSize(labelSize.y + style.FramePadding.y * 2.0f));
				*/
				constexpr uint columnCount = 3;
				constexpr float barHeightFactor = 1.2f;
				const std::string label[columnCount] =
				{
					"Name",
					"Part",
					"Erase",
				};

				const float btnHeight = JGui::CalDefaultButtonSize(label[0]).y * barHeightFactor;
				const JVector2<float> listSize = JGui::GetRestWindowContentsSize() - JVector2<float>(xPadding * 2,  btnHeight * 2);
				const JVector2<float> listPadding = listSize * 0.01f;
				const JVector2<float> contentsSize = JVector2<float>(listSize.x - listPadding.x * 2, btnHeight);
		 
				const JVector2<float> innerSize[columnCount] =
				{
					JVector2<float>{contentsSize.x * 0.45f, contentsSize.y },
					JVector2<float>{contentsSize.x * 0.45f, contentsSize.y },
					JVector2<float>{contentsSize.x * 0.1f, contentsSize.y },
				};

				JGui::SetCursorPosX(JGui::GetCursorPosX() + xPadding);
				JGui::BeginListBox(("##AvatarEditor" + GetName()), listSize);
				JGui::Separator();

				JEditorDynamicAlignCalculator<columnCount> alignCal;
				JEditorTextAlignCalculator textCal;

				alignCal.Update(listSize, contentsSize, listPadding, CreateVec2(0), innerSize, J_EDITOR_INNER_ALGIN_TYPE::ROW, JGui::GetCursorPos());
				for (int i = 0; i < columnCount; ++i)
				{
					alignCal.SetNextContentsPosition(); 
					textCal.Update(label[i], alignCal.GetInnerContentsSize(), false);
					JGui::Text(textCal.LeftAligned());
				}
				for (int i = 0; i < JAvatar::jointGuide[index].size(); ++i)
				{
					alignCal.SetNextContentsPosition();
					textCal.Update(JAvatar::jointGuide[index][i].guideName, alignCal.GetInnerContentsSize(), true);
					JGui::Text(textCal.LeftAligned());

					int jointRefIndex = JAvatar::jointGuide[index][i].index;
					int nowRefValue = targetAvatar.jointReference[jointRefIndex];

					std::string nowRefJointName;
					if (nowRefValue == JSkeletonFixedData::incorrectJointIndex)
						nowRefJointName = "None##" + std::to_string(i);
					else
						nowRefJointName = JCUtil::WstrToU8Str(targetSkeleton->GetSkeleton()->GetJointName(nowRefValue));

					if (!isValidJointRef[jointRefIndex])
						JGui::PushColor(J_GUI_COLOR::BUTTON, failColor);

					JVector2<float> nowSize = alignCal.GetInnerContentsSize();
					alignCal.SetNextContentsPosition();
					textCal.Update(nowRefJointName, alignCal.GetInnerContentsSize(), true);
					if (JGui::Button((textCal.LeftAligned()), nowSize))
					{
						int doJointRefIndex = jointRefIndex;
						int undoJointRefIndex = jointRefIndex;
						std::string taskName;

						//함수 실행시 isOpenJointSelector = !isOpenJointSelector이 되므로
						if (!isOpenJointSelector)
							taskName = "Open avatar joint selector";
						else
							taskName = "Cancel open avatar Joint Selector";
						JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>(taskName,
							"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
							std::make_unique<OpenJointSelectorF::CompletelyBind>(openJointSelectorFunctor, std::move(doJointRefIndex)),
							std::make_unique<OpenJointSelectorF::CompletelyBind>(openJointSelectorFunctor, std::move(undoJointRefIndex))));
					}

					nowSize = alignCal.GetInnerContentsSize();
					alignCal.SetNextContentsPosition();
					if (JGui::Button(("##" + nowRefJointName + "erase"), nowSize - JVector2F(frameBorderSize * 8, 0)))
					{
						int doJointRefIndex = jointRefIndex;
						int doJointIndex = JSkeletonFixedData::incorrectJointIndex;
						int undoJointRefIndex = jointRefIndex;
						int undoJointIndex = targetAvatar.jointReference[jointRefIndex];

						JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Erase avatar joint reference",
							"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
							std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(doJointRefIndex), std::move(doJointIndex)),
							std::make_unique<SetJointRefF::CompletelyBind>(setJointRefFunctor, std::move(undoJointRefIndex), std::move(undoJointIndex))));
					}

					if (!isValidJointRef[jointRefIndex])
						JGui::PopColor();
				}

				JGui::EndListBox();

				JGui::SetCursorPosX(JGui::GetCursorPosX() + xPadding);
				if (JGui::Button("Auto", JVector2<float>(JGui::CalTextSize("_Auto_").x, 0)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Auto generate avatar joint reference",
						"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
						std::make_unique<SetAllJointRefByAutoF::CompletelyBind>(setAllJointRefByAutoFunctor),
						std::make_unique<SetAllJointRefByVecF::CompletelyBind>(setAllJointRefByVecFunctor, std::move(preJointRef))));
				}

				const float btnPaddingRate = 0.0025f;
				const float btnPadding = wndSize.x * btnPaddingRate;

				JGui::SameLine();
				JGui::SetCursorPosX(JGui::GetCursorPosX() + btnPadding);
				if (JGui::Button("Save", JVector2<float>(JGui::CalTextSize("_Save_").x, 0)))
				{
					if (CheckAllJoint())
					{
						StoreAvatarData();
						JEditorTransition::Instance().Log("Store Avatar Data");
					}
				}

				JGui::SameLine();
				JGui::SetCursorPosX(JGui::GetCursorPosX() + btnPadding);
				if (JGui::Button("Clear", JVector2<float>(JGui::CalTextSize("_Clear_").x, 0)))
				{
					std::vector<uint8> preJointRef = targetAvatar.jointReference;
					JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Clear avatar joint reference",
						"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
						std::make_unique<ClearJointRefF::CompletelyBind>(clearJointRefFunctor),
						std::make_unique<SetAllJointRefByVecF::CompletelyBind>(setAllJointRefByVecFunctor, std::move(preJointRef))));
				}

				if (isOpenJointSelector)
				{
					JGui::BeginWindow("Selector", &isOpenJointSelector);
					BuildObjectExplorer(0, targetSkeleton->GetSkeletonTreeIndexVec());
					JGui::EndWindow();
				}
			}
		}
		void JAvatarEditor::BuildObjectExplorer(const uint8 index, const std::vector<std::vector<uint8>>& treeIndexVec)
		{
			std::string name = JCUtil::WstrToU8Str(targetSkeleton->GetJointName(index));
			bool arrowHovered= JGui::IsTreeNodeArrowHovered(name);
			bool isSelected = selectJointIndex == index;

			PushTreeNodeColorSet(true, isSelected);
			bool res = JGui::TreeNodeEx(name.c_str(), J_GUI_TREE_NODE_FLAG_FRAMED | J_GUI_TREE_NODE_FLAG_OPEN_ON_ARROW);
			PopTreeNodeColorSet(true, isSelected);

			if ((JGui::IsLastItemClicked(Core::J_MOUSE_BUTTON::LEFT) || JGui::IsLastItemClicked(Core::J_MOUSE_BUTTON::RIGHT)) && !arrowHovered)
			{
				if (isSelected)
				{
					int doJointRefIndex = selectJointRefIndex;
					int doJointIndex = index;
					int undoJointRefIndex = selectJointRefIndex;
					int undoJointIndex = targetAvatar.jointReference[selectJointRefIndex];
					 
					JEditorTransition::Instance().Execute(std::make_unique<Core::JTransitionSetValueTask>("Select avatar joint reference",
						"skeleton name: " + JCUtil::WstrToU8Str(targetSkeleton->GetName()),
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
				JGui::TreePop();
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
			JUserPtr<JSkeleton> skeleton = targetSkeleton->GetSkeleton();
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
		void JAvatarEditor::StoreAvatarData()
		{
			AvatarInterface::SetAvatar(targetSkeleton.Get(), &targetAvatar); 
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
					AvatarInterface::CopyAvatarJointIndex(targetSkeleton.Get(), &targetAvatar);
					CheckAllJoint();
				}
			}
		}
	}
}