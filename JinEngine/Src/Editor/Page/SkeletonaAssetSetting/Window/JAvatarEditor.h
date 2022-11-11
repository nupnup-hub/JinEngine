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
	namespace Editor
	{
		class JAvatarEditor final : public JEditorWindow
		{ 
		private:
			using MakeAvatarF = Core::JMFunctorType<JAvatarEditor, void>;
			using ClearAvatarF = Core::JMFunctorType<JAvatarEditor, void>;
			using SelectTabF = Core::JMFunctorType<JAvatarEditor, void, const uint>;
			using OpenJointSelectorF = Core::JMFunctorType<JAvatarEditor, void, const int>;
			using SetJointRefF = Core::JMFunctorType<JAvatarEditor, void, const int, const int>;
			using SetAllJointRefByVecF = Core::JMFunctorType<JAvatarEditor, void, std::vector<uint8>&&>;
			using SetAllJointRefByAutoF = Core::JMFunctorType<JAvatarEditor, void>;
			using ClearJointRefF = Core::JMFunctorType<JAvatarEditor, void>;
		private:
			//Functor
			MakeAvatarF::Functor makeAvatarFunctor;
			ClearAvatarF::Functor clearAvatarFunctor;
			SelectTabF::Functor selectTabFunctor;
			OpenJointSelectorF::Functor openJointSelectorFunctor;
			SetJointRefF::Functor setJointRefFunctor;
			SetAllJointRefByVecF::Functor setAllJointRefByVecFunctor;
			SetAllJointRefByAutoF::Functor setAllJointRefByAutoFunctor;
			ClearJointRefF::Functor clearJointRefFunctor;
		private: 
			Core::JUserPtr<JSkeletonAsset> targetSkeleton; 
			JAvatar targetAvatar;

			bool hasAvatar = false;
			bool isOpenAvatarSetting = false;
			bool isOpenJointSelector = false; 
			int selectJointRefIndex; 
			int selectJointIndex;
			std::bitset<JSkeletonFixedData::maxAvatarJointCount> isValidJointRef; 
			std::bitset<4> tabs;
			const JVector4<float> failColor{ 0.85f, 0.2f, 0.2f, 0.75f };
		public:
			JAvatarEditor(const std::string& name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_TYPE ownerPageType);
			~JAvatarEditor();
			JAvatarEditor(const JAvatarEditor & rhs) = delete;
			JAvatarEditor& operator=(const JAvatarEditor & rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:			  
			void Initialize(const Core::JUserPtr<JSkeletonAsset>& newTargetSkeleton)noexcept;
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
			bool ArrowClick(const std::string& name); 
			void StoreAvatarData();
		private:
			void DoActivate()noexcept final;
		};
	}
}