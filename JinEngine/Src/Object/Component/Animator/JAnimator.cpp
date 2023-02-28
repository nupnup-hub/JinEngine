#include"JAnimator.h"
#include"../JComponentFactory.h"
#include"../../Resource/AnimationController/JAnimationController.h" 
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/Skeleton/JSkeletonFixedData.h"
#include"../../Resource/JResourceManager.h" 
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/FSM/JFSMparameter.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationUpdateData.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h"
#include<fstream>

namespace JinEngine
{
	static auto isAvailableoverlapLam = []() {return false; };

	J_COMPONENT_TYPE JAnimator::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
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
		if(IsActivated())
			CallOffResourceReference(skeletonAsset);
		skeletonAsset = newSkeletonAsset;
		if (IsActivated())
			CallOnResourceReference(skeletonAsset);

		ReRegisterComponent(); 
	}
	void JAnimator::SetAnimatorController(JAnimationController* newAnimationController)noexcept
	{
		if (IsActivated())
			CallOffResourceReference(animationController);
		animationController = newAnimationController;
		if (IsActivated())
			CallOnResourceReference(animationController);

		ReRegisterComponent(); 
		
		//act scene timer
		if (userTimer && IsActivated())
		{
			ClearAnimationUpdateData();
			reqSettingAniData = true;
			SettingAnimationUpdateData();
		}
	}
	void JAnimator::SetParameterValue(const std::wstring& paramName, const float value)noexcept
	{
		if (animationUpdateData != nullptr)
			animationUpdateData->SetParameterValue(paramName, value);
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
	JAnimatorFrameUpdateTriggerInterface* JAnimator::UpdateTriggerInterface()
	{
		return this;
	}
	void JAnimator::OnAnimationUpdate(Core::JGameTimer* sceneTimer)noexcept
	{
		reqSettingAniData = true;
		userTimer = sceneTimer;
		SettingAnimationUpdateData();
	}
	void JAnimator::OffAnimationUpdate()noexcept
	{
		reqSettingAniData = false;
		userTimer = nullptr;
		ClearAnimationUpdateData();
	}
	void JAnimator::SettingAnimationUpdateData()noexcept
	{
		if (reqSettingAniData && IsActivated() && animationController)
		{
			animationUpdateData = std::make_unique<Core::JAnimationUpdateData>();
			animationUpdateData->Initialize();
			animationUpdateData->timer = userTimer;
			animationUpdateData->modelSkeleton = skeletonAsset;
		 
			const uint paramCount = animationController->GetParameterCount();
			for (uint i = 0; i < paramCount; ++i)
			{
				Core::JFSMparameter* param = animationController->GetParameterByIndex(i);
				animationUpdateData->RegisterParameter(param->GetName(), param->GetValue());
			}

			animationController->FrameUpdateInterface()->Initialize(animationUpdateData.get());
			reqSettingAniData = false;
		}
	}
	void JAnimator::ClearAnimationUpdateData()noexcept
	{
		animationUpdateData.reset();
	}
	bool JAnimator::CanUpdateAnimation()const noexcept
	{
		return animationController != nullptr && userTimer != nullptr;
	}
	void JAnimator::DoCopy(JObject* ori)
	{
		JAnimator* oriAni = static_cast<JAnimator*>(ori);
		SetSkeletonAsset(oriAni->skeletonAsset);
		SetAnimatorController(oriAni->animationController); 
	}
	void JAnimator::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent(); 
		CallOnResourceReference(skeletonAsset);
		CallOnResourceReference(animationController);
		SettingAnimationUpdateData();
	}
	void JAnimator::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent(); 
		CallOffResourceReference(skeletonAsset);
		CallOffResourceReference(animationController);
		ClearAnimationUpdateData();
	}
	void JAnimator::UpdateFrame(Graphic::JAnimationConstants& constant)
	{
		if (animationController != nullptr)
		{
			animationUpdateData->timer = userTimer;
			animationUpdateData->modelSkeleton = skeletonAsset;
			animationController->FrameUpdateInterface()->Update(animationUpdateData.get(), constant);
		}
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
		  
		JFileIOHelper::StoreObjectIden(stream, animator);
		JFileIOHelper::StoreHasObjectIden(stream ,animator->GetAnimatorController());
		JFileIOHelper::StoreHasObjectIden(stream, animator->GetSkeletonAsset());

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JAnimator* JAnimator::LoadObject(std::wifstream& stream, JGameObject* owner)
	{
		if (owner == nullptr)
			return nullptr;

		if (!stream.is_open() || stream.eof())
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		Core::JIdentifier* aniCont = JFileIOHelper::LoadHasObjectIden(stream);
		Core::JIdentifier* skeletonAsset = JFileIOHelper::LoadHasObjectIden(stream);
 
		Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(guid, flag, owner);
		JAnimator* newAnimator = ownerPtr.Get();
		if (!AddInstance(std::move(ownerPtr)))
			return nullptr;

		if (aniCont != nullptr && aniCont->GetTypeInfo().IsA(JAnimationController::StaticTypeInfo()))
			newAnimator->SetAnimatorController(static_cast<JAnimationController*>(aniCont));
		if (skeletonAsset != nullptr && skeletonAsset->GetTypeInfo().IsA(JSkeletonAsset::StaticTypeInfo()))
			newAnimator->SetSkeletonAsset(static_cast<JSkeletonAsset*>(skeletonAsset));
		return newAnimator;
	}
	void JAnimator::RegisterJFunc()
	{
		auto defaultC = [](JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(Core::MakeGuid(), owner->GetFlag(), owner);
			JAnimator* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto initC = [](const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)-> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(guid, Core::AddSQValueEnum(owner->GetFlag(), objFlag), owner);
			JAnimator* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
				return newComp;
			else
				return nullptr;
		};
		auto loadC = [](std::wifstream& stream, JGameObject* owner) -> JComponent*
		{
			return LoadObject(stream, owner);
		};
		auto copyC = [](JComponent* ori, JGameObject* owner) -> JComponent*
		{
			Core::JOwnerPtr ownerPtr = JPtrUtil::MakeOwnerPtr<JAnimator>(Core::MakeGuid(), ori->GetFlag(), owner);
			JAnimator* newComp = ownerPtr.Get();
			if (AddInstance(std::move(ownerPtr)))
			{
				if (newComp->Copy(ori))
					return newComp;
				else
				{
					BegineForcedDestroy(newComp);
					return nullptr;
				}
			}
			else
				return nullptr;
		};
		JCFI<JAnimator>::Register(defaultC, initC, loadC, copyC);

		static GetTypeNameCallable getTypeNameCallable{ &JAnimator::TypeName };
		static GetTypeInfoCallable getTypeInfoCallable{ &JAnimator::StaticTypeInfo };
		bool(*ptr)() = isAvailableoverlapLam;
		static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

		//static auto setFrameLam = [](JComponent& component) {static_cast<JAnimator*>(&component)->SetFrameDirty(); };
		//static SetFrameDirtyCallable setFrameDirtyCallable{ setFrameLam };

		static JCI::CTypeHint cTypeHint{ GetStaticComponentType(), true };
		static JCI::CTypeCommonFunc cTypeCommonFunc{ getTypeNameCallable, getTypeInfoCallable, isAvailableOverlapCallable };
		static JCI::CTypeInterfaceFunc cTypeInterfaceFunc{ nullptr, nullptr };

		JCI::RegisterTypeInfo(cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
	}
	JAnimator::JAnimator(const size_t guid, const J_OBJECT_FLAG objFlag, JGameObject* owner)
		:JAnimatorInterface(TypeName(), guid, objFlag, owner)
	{ 
		AddEventListener(*JResourceManager::Instance().EvInterface(), GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	JAnimator::~JAnimator()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), GetGuid());
	}
}