#include"JAnimator.h"
#include"../JComponentFactory.h"
#include"../../Resource/AnimationController/JAnimationController.h" 
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/Skeleton/JSkeletonFixedData.h"
#include"../../Resource/JResourceManager.h"
#include"../../GameObject/JGameObject.h"
#include"../../GameObject/IGameObjectComponentEvent.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
 
namespace JinEngine
{
	void JAnimator::OnAnimation()noexcept
	{
		if (animationController != nullptr)
			animationController->Initialize(animationTimes, skeletonAsset);
	}
	void JAnimator::Update(Graphic::JAnimationConstants& animationConstatns, bool isOnAnimation)
	{
		if (animationController != nullptr && isOnAnimation)
			animationController->Update(animationTimes, skeletonAsset, animationConstatns);
		else
		{
			DirectX::XMFLOAT4X4 iden = JMathHelper::Identity4x4();
			for (uint i = 0; i < JSkeletonFixedData::maxJointCount; ++i)
				animationConstatns.boneTransforms[i] = iden;
		}
	}
	JAnimationController* JAnimator::GetAnimatorController()const noexcept
	{
		return animationController;
	}
	uint JAnimator::GetAnimationCBIndex()const noexcept
	{
		return aniCBIndex;
	}
	JSkeletonAsset* JAnimator::GetSkeletonAsset()noexcept
	{
		return skeletonAsset;
	}
	void JAnimator::SetAnimatorController(JAnimationController* animationController)noexcept
	{
		JAnimator::animationController = animationController;
		ReRegisterComponent<JAnimator>(this);
		SetDirty();
	}
	void JAnimator::SetAnimationCBIndex(const uint index)noexcept
	{
		aniCBIndex = index;
	}
	void JAnimator::SetSkeletonAsset(JSkeletonAsset* newSkeletonAsset)noexcept
	{
		skeletonAsset = newSkeletonAsset;
		ReRegisterComponent<JAnimator>(this);
		SetDirty();
	}
	bool JAnimator::IsDirtied()const noexcept
	{
		return gameObjectDirty->GetAnimatorDirty() > 0;
	}
	void JAnimator::SetDirty()noexcept
	{
		gameObjectDirty->SetAnimatorDirty();
	}
	void JAnimator::OffDirty()noexcept
	{
		gameObjectDirty->OffAnimatorDirty();
	}
	void JAnimator::MinusDirty()noexcept
	{
		gameObjectDirty->AnimatorUpdate();
	}
	J_COMPONENT_TYPE JAnimator::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	J_COMPONENT_TYPE JAnimator::GetStaticComponentType()noexcept
	{
		return  J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR;
	}
	bool JAnimator::IsAvailableOverlap()const noexcept
	{
		return false;
	}
	bool JAnimator::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && skeletonAsset != nullptr && animationController != nullptr)
			return true;
		else
			return false;
	}
	void JAnimator::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent<JAnimator>(this);
		SetDirty();
	}
	void JAnimator::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent<JAnimator>(this);
		OffDirty();
	}
	Core::J_FILE_IO_RESULT JAnimator::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JAnimator::StoreObject(std::wofstream& stream, JAnimator* animator)
	{
		if(animator == nullptr)
			return Core::J_FILE_IO_RESULT::FAIL_NULL_OBJECT;

		if (((int)animator->GetFlag() & OBJECT_FLAG_DO_NOT_SAVE) > 0)
			return Core::J_FILE_IO_RESULT::FAIL_DO_NOT_SAVE_DATA;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		Core::J_FILE_IO_RESULT res = StoreMetadata(stream, animator);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		JAnimationController* aniController = animator->GetAnimatorController();
		if (aniController != nullptr)
			stream << true << " " << aniController->GetGuid() << " ";
		else
			stream << false << " " << 0 << " ";

		JSkeletonAsset* skeletonAsset = animator->GetSkeletonAsset();
		if (skeletonAsset != nullptr)
			stream << true << " " << skeletonAsset->GetGuid() << '\n';
		else
			stream << false << " " << 0 << '\n';

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JAnimator* JAnimator::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open())
			return nullptr;
		 
		ObjectMetadata metadata;
		Core::J_FILE_IO_RESULT loadMetaRes = LoadMetadata(stream, metadata);
		 
		JAnimator* newAnimator;
		if (loadMetaRes == Core::J_FILE_IO_RESULT::SUCCESS)
			newAnimator = new JAnimator(metadata.guid, metadata.flag, owner);
		else
			newAnimator = new JAnimator(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);

		bool hasAniController;
		bool hasSkeletonAsset;
		size_t aniControllerGuid;
		size_t skeletonAssetGuid;
		stream >> hasAniController >> aniControllerGuid >> hasSkeletonAsset >> skeletonAssetGuid;

		if (hasAniController)
		{
			JAnimationController* aniController = JResourceManager::Instance().GetResource<JAnimationController>(aniControllerGuid);
			if (aniController != nullptr)
				newAnimator->SetAnimatorController(aniController);
		}
		if (hasSkeletonAsset)
		{
			JSkeletonAsset* skeletonAsset = JResourceManager::Instance().GetResource<JSkeletonAsset>(skeletonAssetGuid);
			if (skeletonAsset != nullptr)
				newAnimator->SetSkeletonAsset(skeletonAsset);
		}
		return newAnimator;
	}
	void JAnimator::RegisterFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			return new JAnimator(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
		};
		auto initC = [](const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			return new JAnimator(guid, objFlag, owner);
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			JAnimator* oriAni = static_cast<JAnimator*>(ori);
			JAnimator* newAni = new JAnimator(Core::MakeGuid(), oriAni->GetFlag(), owner);
			
			newAni->SetSkeletonAsset(oriAni->skeletonAsset);
			newAni->SetAnimatorController(oriAni->animationController); 

			return newAni;
		};
		JCFI<JAnimator>::Regist(defaultC, initC, loadC, copyC);
	}
	JAnimator::JAnimator(const size_t guid, const JOBJECT_FLAG objFlag, JGameObject* owner)
		:JComponent(TypeName(), guid, objFlag, owner)
	{
		gameObjectDirty = owner->GetGameObjectDirty();
		animationTimes.resize(JAnimationController::diagramMaxCount);
	}
	JAnimator::~JAnimator()
	{
		gameObjectDirty = nullptr;
		skeletonAsset = nullptr;
		animationController = nullptr;
	}
}