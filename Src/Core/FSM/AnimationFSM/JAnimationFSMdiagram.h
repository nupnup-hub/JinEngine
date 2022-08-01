#pragma once
#include"../JFSMdiagram.h" 
#include"JBlender.h" 
#include"JAnimationPostProcessing.h"
#include"JAnimationRetargeting.h"

namespace JinEngine
{
	class JAnimationClip;
	class JSkeletonAsset;
	struct EditorDiagram;
	namespace Graphic
	{
		struct JAnimationConstants;
	}

	namespace Core
	{
		struct JAnimationShareData;
		struct JAnimationTime;
		class JAnimationFSMstate;
		class JAnimationFSMtransition;
		class JAnimationFSMdiagram : public JFSMdiagram
		{
		private:
			std::unique_ptr<EditorDiagram> editorDiagram;
			std::vector<JAnimationFSMstate*> stateCash;
			JAnimationFSMstate* nowState;
			JAnimationFSMstate* nextState;
			JAnimationFSMtransition* nextTransition;
			JBlender blender;
			float weight;
		public:
			JAnimationFSMdiagram(const std::string& name, IJFSMconditionStorageUser* conditionStorage);
			~JAnimationFSMdiagram();
			void Initialize(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
			void Enter(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset);
			void Update(JAnimationTime& animationTime, JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns, const uint layerNumber)noexcept;
			bool HasAnimationData()noexcept;

			std::vector<JAnimationFSMstate*>::const_iterator GetAnimationFSMstateVectorHandle(_Out_ uint& stateCount)noexcept;
			EditorDiagram* GetEditorDiagram()noexcept;

			void SetStateName(const std::string& oldName, const std::string& newName);
			void SetAnimationClip(const std::string& stateName, JAnimationClip* clip);
			void SetTransitionCondition(const std::string& stateName, const std::string& outputStateName, const std::string& newConditionName, const uint oldConditionIndex)noexcept;
			void SetTransitionCondtionOnValue(const std::string& stateName, const std::string& outputStateName, const uint conditionIndex, const float value)noexcept;

			JAnimationFSMstate* AddAnimationClipState()noexcept;
			JAnimationFSMtransition* AddAnimationTransition(const std::string& stateName, const std::string& outputStateName)noexcept;

			bool EraseAnimationState(const std::string& stateName)noexcept;
			bool EraseAnimationTransition(const std::string& stateName, const std::string& outputStateName)noexcept;
			void Clear()noexcept;

			bool HasState()noexcept;
			void StuffFinalTransform(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
			void CrossFading(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset, Graphic::JAnimationConstants& animationConstatns)noexcept;
		private:
			void PreprocessSkeletonBindPose(JAnimationShareData& animationShareData, JSkeletonAsset* srcSkeletonAsset)noexcept;
		};
	}
}