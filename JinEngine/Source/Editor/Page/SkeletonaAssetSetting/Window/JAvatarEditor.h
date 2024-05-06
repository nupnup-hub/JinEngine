/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"../../JEditorWindow.h" 
#include"../../../../Object/JObjectModifyInterface.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../../Object/Resource/Skeleton/Avatar/JAvatar.h"
#include"../../../../Core/Math/JVector.h" 

namespace JinEngine
{
	class JGameObject; 
	class JAvatar;
	class JSkeletonAsset;
	namespace Editor
	{
		class JEditorTreeStructure; 
		class JAvatarEditorSettingFunctor;
		class JAvatarEditor final : public JEditorWindow, public JObjectModifyInterface
		{ 
		private: 
			std::unique_ptr<JAvatarEditorSettingFunctor> setting;
		private:
			std::unique_ptr<JEditorTreeStructure> treeStrcture;
		private: 
			JUserPtr<JSkeletonAsset> targetSkeleton; 
			JAvatar targetAvatar;
		private:
			bool hasAvatar = false;
			bool isOpenAvatarSetting = false;
			bool isOpenJointSelector = false; 
			int selectJointRefIndex; 
			int selectJointIndex;
			std::bitset<JSkeletonFixedData::maxAvatarJointCount> isValidJointRef; 
			std::bitset<4> tabs;
			const JVector4<float> failColor{ 0.85f, 0.2f, 0.2f, 0.75f }; 
		public:
			JAvatarEditor(const std::string& name,
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JAvatarEditor();
			JAvatarEditor(const JAvatarEditor & rhs) = delete;
			JAvatarEditor& operator=(const JAvatarEditor & rhs) = delete;
		private:
			void InitializeSettingImpl();
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:			  
			void Initialize(const JUserPtr<JSkeletonAsset>& newTargetSkeleton)noexcept;
			void UpdateWindow()final;
		private:
			//Build Gui
			void BuildAvatarEdit();
			void BuildObjectExplorer(const uint8 index, const std::vector<std::vector<uint8>>& treeIndexVec);
			//void BuildObjectExplorer(JGameObject* obj, uint index);
		private:
			void MakeAvatar();
			void ClearAvatar();
			void SelectTab(const uint index);
			void OpenJointSelector(const int jointRefIndx);
			void SetJointReference(const int jointRefIndex, const int jointIndex);
			void SetAllJointReferenceByVec(std::vector<uint8>&& vec); 
			void SetAllJointReferenceByAuto();
			void ClearJointReference();
		private:
			void CloseAllTab()noexcept;
			int FindSelectedTab()noexcept;
			bool CheckAllJoint()noexcept;
			bool IsAllJointPassed()noexcept; 
			void StoreAvatarData();
		private:
			void DoActivate()noexcept final;
		private:
			void LoadEditorWindow(JFileIOTool& tool)final;
			void StoreEditorWindow(JFileIOTool& tool)final;
		};
	}
}