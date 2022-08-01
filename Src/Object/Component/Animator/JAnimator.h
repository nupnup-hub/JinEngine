#pragma once
#include"../JComponent.h" 
#include"../../IFrameResourceControl.h" 
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include<memory>
#include<vector>
#include<DirectXMath.h>

namespace JinEngine
{  
	class JSkeletonAsset;
	class JAnimationClip;
	class JAnimationController;
	class GameObjectDirty;

	namespace Graphic
	{
		struct JAnimationConstants;
	}

	class JAnimator : public JComponent, public IFrameResourceControl
	{
		REGISTER_CLASS(JAnimator)
	private:
		GameObjectDirty* gameObjectDirty;
		JSkeletonAsset* skeletonAsset;
		JAnimationController* animationController;
		std::vector<Core::JAnimationTime>animationTimes;
		uint aniCBIndex;
	public:
		void OnAnimation()noexcept;
		void Update(Graphic::JAnimationConstants& animationConstatns, bool isOnAnimation);
		JAnimationController* GetAnimatorController()const noexcept;
		uint GetAnimationCBIndex()const noexcept;
		JSkeletonAsset* GetSkeletonAsset()noexcept;

		void SetAnimatorController(JAnimationController* animationController)noexcept;
		void SetAnimationCBIndex(const uint index)noexcept;
		void SetSkeletonAsset(JSkeletonAsset* newSkeletonAsset)noexcept;

		bool IsDirtied()const noexcept;
		void SetDirty()noexcept;
		void OffDirty()noexcept;
		void MinusDirty()noexcept;

		J_COMPONENT_TYPE GetComponentType()const noexcept final;
		static J_COMPONENT_TYPE GetStaticComponentType()noexcept;
		bool IsAvailableOverlap()const noexcept final;
		bool PassDefectInspection()const noexcept final; 
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		Core::J_FILE_IO_RESULT CallStoreComponent(std::wofstream& stream)final;
		static Core::J_FILE_IO_RESULT StoreObject(std::wofstream& stream, JAnimator* animator);
		static JAnimator* LoadObject(std::wifstream& stream, JGameObject* owner);
		static void RegisterFunc();
	private:
		JAnimator(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner);
		~JAnimator();
	};
}