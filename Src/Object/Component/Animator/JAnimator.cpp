#include"JAnimator.h"
#include"../JComponentFactory.h"
#include"../../Resource/AnimationController/JAnimationController.h" 
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/Skeleton/JSkeletonFixedData.h"
#include"../../Resource/JResourceManager.h"
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"

namespace JinEngine
{
	static auto isAvailableoverlapLam = []() {return false; };
	static auto componentTypeLam = []() {return J_COMPONENT_TYPE::ENGINE_DEFIENED_ANIMATOR; };

	void JAnimator::OnAnimation()noexcept
	{
		if (animationController != nullptr)
			animationController->Initialize(animationTimes, skeletonAsset);
	}
	JSkeletonAsset* JAnimator::GetSkeletonAsset()noexcept
	{
		return skeletonAsset;
	}
	JAnimationController* JAnimator::GetAnimatorController()const noexcept
	{
		return animationController;
	}
	void JAnimator::SetSkeletonAsset(JSkeletonAsset* newSkeletonAsset)noexcept
	{
		if (skeletonAsset != nullptr)
			OffResourceReference(*skeletonAsset);
		skeletonAsset = newSkeletonAsset;
		if (skeletonAsset != nullptr)
			OnResourceReference(*skeletonAsset);
		ReRegisterComponent();
		SetFrameDirty();
	}
	void JAnimator::SetAnimatorController(JAnimationController* animationController)noexcept
	{
		if (JAnimator::animationController != nullptr)
			OffResourceReference(*JAnimator::animationController);

		JAnimator::animationController = animationController;
		if (JAnimator::animationController != nullptr)
			OnResourceReference(*JAnimator::animationController);
		ReRegisterComponent();
		SetFrameDirty();
	}
	J_COMPONENT_TYPE JAnimator::GetComponentType()const noexcept
	{
		return componentTypeLam();
	}
	bool JAnimator::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JAnimator::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && skeletonAsset != nullptr && animationController != nullptr)
			return true;
		else
			return false;
	}
	bool JAnimator::Copy(JObject* ori)
	{
		if (ori->HasFlag(OBJECT_FLAG_UNCOPYABLE) || ori->GetGuid() == GetGuid())
			return false;

		if (typeInfo.IsA(ori->GetTypeInfo()))
		{
			JAnimator* oriAni = static_cast<JAnimator*>(ori);
			SetSkeletonAsset(oriAni->skeletonAsset);
			SetAnimatorController(oriAni->animationController);
			SetFrameDirty();
			return true;
		}
		else
			return false;
	}
	void JAnimator::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent();
		SetFrameDirty();
	}
	void JAnimator::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		OffFrameDirty();
	}
	bool JAnimator::UpdateFrame(Graphic::JAnimationConstants& constant)
	{
		if (IsFrameDirted() && animationController != nullptr)
		{
			animationController->Update(animationTimes, skeletonAsset, constant);
			MinusFrameDirty();
			return true;
		}
		else
			return false;
	}
	void JAnimator::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == GetGuid())
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (skeletonAsset != nullptr && skeletonAsset->GetGuid() == jRobj->GetGuid())
				SetSkeletonAsset(nullptr);
			else if (animationController != nullptr && animationController->GetGuid() == jRobj->GetGuid())
				SetAnimatorController(nullptr);			 
		}
	}
	Core::J_FILE_IO_RESULT JAnimator::CallStoreComponent(std::wofstream& stream)
	{
		return StoreObject(stream, this);
	}
	Core::J_FILE_IO_RESULT JAnimator::StoreObject(std::wofstream& stream, JAnimator* animator)
	{
		if (animator == nullptr)
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
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(metadata.guid, metadata.flag, owner);
			JAnimator* newAnimator = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
		}
		else
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JAnimator* newAnimator = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
		}
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
	void JAnimator::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(Core::MakeGuid(), OBJECT_FLAG_NONE, owner);
			JComponent* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(guid, objFlag, owner);
			JComponent* ret = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			return ret; 
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(Core::MakeGuid(), ori->GetFlag(), owner);
			JAnimator* newAni = ownerPtr.Get();
			AddInstance(std::move(ownerPtr));
			newAni->Copy(ori);
			return newAni;
		};
		JCFI<JAnimator>::Regist(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JAnimator::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JAnimator::StaticTypeInfo };	 
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		static auto setFrameLam = [](JComponent& component){static_cast<JAnimator*>(&component)->SetFrameDirty();};
		static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ componentTypeLam(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable, isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ &setFrameDirtyCallable };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JAnimator::JAnimator(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JAnimatorInterface(TypeName(), guid, objFlag, owner)
	{
		animationTimes.resize(JAnimationController::diagramMaxCount);
	}
	JAnimator::~JAnimator()
	{
		skeletonAsset = nullptr;
		animationController = nullptr;
	}
}