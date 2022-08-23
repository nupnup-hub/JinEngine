#pragma once 
#include"../../JEditorWindow.h" 
#include"../../../../Object/Resource/Skeleton/Avatar/JAvatar.h"
#include"../../../../Object/Resource/Skeleton/JSkeletonFixedData.h"
#include"../../../../Utility/JVector.h"
#include<vector>
#include<bitset>

namespace JinEngine
{
	class JGameObject;
	class JModel;
	namespace Editor
	{
		class JAvatarEditor : public JEditorWindow
		{
		private: 
			size_t modelGuid;
			JAvatar targetAvatar;

			bool hasAvatar = false;
			bool isOpenAvatarSetting = false;
			bool isOpenJointSelector = false; 
			int selectJointRefIndex;
			int selectJointIndex;
			std::bitset<JSkeletonFixedData::maxAvatarJointCount> isValidJointRef; 
			std::bitset<4> tabs;
			const JVector4<float> failColor{ 0.85f, 0.2f, 0.2f, 0.75f };
		private:
			//Functor
			Core::JFunctor<void> makeAvatarFunctor;
			Core::JFunctor<void> clearAvatarFunctor;
			Core::JFunctor<void, const uint> selectTabFunctor;
			Core::JFunctor<void, const int> openJointSelectorFunctor;
			Core::JFunctor<void, const int, const int> setJointRefFunctor; 
			Core::JFunctor<void, std::vector<uint8>&&> setAllJointRefByVecFunctor; 
			Core::JFunctor<void> setAllJointRefByAutoFunctor;
			Core::JFunctor<void> clearJointRefFunctor;
		public:
			JAvatarEditor(std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JAvatarEditor();
			JAvatarEditor(const JAvatarEditor & rhs) = delete;
			JAvatarEditor& operator=(const JAvatarEditor & rhs) = delete;
		public:			  
			void UpdateWindow()final;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept;
			void SetModelGuid(const size_t guid)noexcept;
		private:
			//Build Gui
			void BuildAvatarEdit();
			void BuildObjectExplorer(JGameObject* obj, uint index);
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
			bool ArrowClick(const std::string& name); 
			void StoreAvatarData();
		private:
			void DoActivate()noexcept final;
		};
	}
}