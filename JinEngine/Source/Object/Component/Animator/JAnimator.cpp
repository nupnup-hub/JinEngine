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


#include"JAnimator.h"
#include"JAnimatorPrivate.h" 
#include"../JComponentHint.h"
#include"../../Resource/AnimationController/JAnimationController.h"
#include"../../Resource/AnimationController/JAnimationControllerPrivate.h" 
#include"../../Resource/AnimationController/FSM/JAnimationTime.h"
#include"../../Resource/AnimationController/FSM/JAnimationUpdateData.h"
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/Skeleton/JSkeletonFixedData.h" 
#include"../../Resource/JResourceObject.h" 
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/JResourceObjectUserInterface.h" 
#include"../../GameObject/JGameObject.h" 
#include"../../JObjectFileIOHelper.h"
#include"../../../Core/FSM/JFSMparameter.h" 
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/Guid/JGuidCreator.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Reflection/JTypeImplBase.h"
#include"../../../Graphic/Frameresource/JAnimationConstants.h" 
#include"../../../Graphic/Frameresource/JFrameUpdate.h"
#include<fstream>
namespace JinEngine
{
	namespace
	{
		using ContFrameUpdateInteface = JAnimationControllerPrivate::FrameUpdateInterface;
		using AnimatorFrameUpdate = Graphic::JFrameUpdate<Graphic::JFrameUpdateInterfaceHolder1<Graphic::JFrameUpdateInterface<Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, Graphic::JAnimationConstants&>>, Core::JEmptyType>;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JAnimatorPrivate aPrivate;
	} 
 
	class JAnimator::JAnimatorImpl : public Core::JTypeImplBase,
		public JResourceObjectUserInterface,
		public AnimatorFrameUpdate
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimatorImpl)
	public:
		using AniFrame = JFrameInterface1;
	public:
		JWeakPtr<JAnimator> thisPointer;
	public:
		REGISTER_PROPERTY_EX(skeletonAsset, GetSkeletonAsset, SetSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false, false))
		JUserPtr<JSkeletonAsset> skeletonAsset;
		REGISTER_PROPERTY_EX(animationController, GetAnimatorController, SetAnimatorController, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false, false))
		JUserPtr<JAnimationController> animationController;
		Core::JGameTimer* userTimer = nullptr;
		std::unique_ptr<JAnimationUpdateData> animationUpdateData;
	public:
		bool reqSettingAniData = false;
	public:
		JAnimatorImpl(const InitData& initData, JAnimator* thisAnimatorRaw)
		{}
		~JAnimatorImpl(){ }
	public:
		JUserPtr<JSkeletonAsset>GetSkeletonAsset()const noexcept
		{
			return skeletonAsset;
		}
		JUserPtr<JAnimationController> GetAnimatorController()const noexcept
		{
			return animationController;
		}
	public:
		void SetSkeletonAsset(JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept
		{
			if (thisPointer->IsActivated())
				CallOffResourceReference(skeletonAsset.Get());
			skeletonAsset = newSkeletonAsset;
			if (thisPointer->IsActivated())
				CallOnResourceReference(skeletonAsset.Get());
			 
			ReRegisterComponent(thisPointer);
		}
		void SetAnimatorController(JUserPtr<JAnimationController> newAnimationController)noexcept
		{
			if (thisPointer->IsActivated())
				CallOffResourceReference(animationController.Get());
			animationController = newAnimationController;
			if (thisPointer->IsActivated())
				CallOnResourceReference(animationController.Get());

			ReRegisterComponent(thisPointer);

			//act scene timer
			if (userTimer && thisPointer->IsActivated())
			{
				ClearAnimationUpdateData();
				reqSettingAniData = true;
				SettingAnimationUpdateData();
			}
		}
		void SettingAnimationUpdateData()noexcept
		{
			if (reqSettingAniData && thisPointer->IsActivated() && animationController.IsValid())
			{
				animationUpdateData = std::make_unique<JAnimationUpdateData>();
				animationUpdateData->Initialize();
				animationUpdateData->timer = userTimer;
				animationUpdateData->modelSkeleton = skeletonAsset;

				const uint paramCount = animationController->GetParameterCount();
				for (uint i = 0; i < paramCount; ++i)
				{
					JUserPtr<Core::JFSMparameter> param = animationController->GetParameterByIndex(i);
					animationUpdateData->RegisterParameter(param->GetGuid(), param->GetValue());
				}
				ContFrameUpdateInteface::Initialize(animationController.Get(), animationUpdateData.get());
				reqSettingAniData = false; 
			}
		}
	public:
		bool CanUpdateAnimation()const noexcept
		{
			return animationController.IsValid() && userTimer != nullptr;
		}
	public:
		void UpdateFrame(Graphic::JAnimationConstants& constant)noexcept final
		{
			animationUpdateData->timer = userTimer;
			animationUpdateData->modelSkeleton = skeletonAsset;
			ContFrameUpdateInteface::Update(animationController.Get(), animationUpdateData.get(), constant);
			AniFrame::MinusMovedDirty();
		}
	public:
		static bool DoCopy(JAnimator* from, JAnimator* to)
		{ 
			to->SetSkeletonAsset(from->GetSkeletonAsset());
			to->SetAnimatorController(from->GetAnimatorController());
			return true;
		}
	public:
		void ClearAnimationUpdateData()noexcept
		{
			animationUpdateData.reset();
		}
	public:
		void OnResourceRef()
		{
			if (skeletonAsset.IsValid())
				CallOnResourceReference(skeletonAsset.Get());
			if (animationController.IsValid())
				CallOnResourceReference(animationController.Get());
		}
		void OffResourceRef()
		{
			if (skeletonAsset.IsValid())
				CallOffResourceReference(skeletonAsset.Get());
			if (animationController.IsValid())
				CallOffResourceReference(animationController.Get());
		}
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
		{
			if (iden == thisPointer->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (skeletonAsset.IsValid() && skeletonAsset->GetGuid() == jRobj->GetGuid())
					SetSkeletonAsset(JUserPtr<JSkeletonAsset>{});
				else if (animationController.IsValid() && animationController->GetGuid() == jRobj->GetGuid())
					SetAnimatorController(JUserPtr<JAnimationController>{});
			}
		}
	public:
		void NotifyReAlloc()
		{
			ReRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, this);
			ResetEventListenerPointer(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
	public:
		void RegisterThisPointer(JAnimator* aniCont)
		{
			thisPointer = Core::GetWeakPtr(aniCont);
		}
		void RegisterPostCreation()
		{
			AddEventListener(*JResourceObject::EvInterface(), thisPointer->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		void DeRegisterPreDestruction()
		{
			RemoveListener(*JResourceObject::EvInterface(), thisPointer->GetGuid());
		}
		void RegisterAnimationFrameData()
		{
			RegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, this, thisPointer->GetOwner()->GetOwnerGuid());
		}
		void DeRegisterAnimationFrameData()
		{
			DeRegisterFrameData(Graphic::J_UPLOAD_FRAME_RESOURCE_TYPE::ANIMATION, this);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JAnimator::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](const Core::JTypeInfo& typeInfo, JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JAnimator::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JAnimator::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), true };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };
			static CTypePrivateFunc cTypeInterfaceFunc{ nullptr};
 
			JComponent::RegisterCTypeInfo(JAnimator::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JAnimator::StaticTypeInfo(), aPrivate);
 
			IMPL_REALLOC_BIND(JAnimator::JAnimatorImpl, thisPointer)
		}
	};

	JAnimator::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JAnimator::StaticTypeInfo(), owner)
	{}
	JAnimator::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JAnimator::StaticTypeInfo(), GetDefaultName(JAnimator::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JAnimator::PrivateInterface()const noexcept
	{
		return aPrivate;
	}
	J_COMPONENT_TYPE JAnimator::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	JUserPtr<JSkeletonAsset> JAnimator::GetSkeletonAsset()const noexcept
	{
		return impl->GetSkeletonAsset();
	}
	JUserPtr<JAnimationController> JAnimator::GetAnimatorController()const noexcept
	{
		return impl->GetAnimatorController();
	}
	void JAnimator::SetSkeletonAsset(JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept
	{
		impl->SetSkeletonAsset(newSkeletonAsset);
	}
	void JAnimator::SetAnimatorController(JUserPtr<JAnimationController> newAnimationController)noexcept
	{
		impl->SetAnimatorController(newAnimationController);
	}
	void JAnimator::SetParameterValue(Core::JFSMparameter* param, const float value)noexcept
	{
		if (impl->animationUpdateData != nullptr)
			impl->animationUpdateData->SetParameterValue(param->GetGuid(), value);
	}
	bool JAnimator::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JAnimator::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection() && impl->skeletonAsset.IsValid() && impl->animationController.IsValid())
			return true;
		else
			return false;
	}
	void JAnimator::DoActivate()noexcept
	{
		//Caution 
		//Activate와 RegisterComponent는 순서에 종속성을 가진다.
		//RegisterComponent는 Scene과 가속구조에 Component에 대한 정보를 추가하는 작업으로
		//Activate Process중에 자기자신과 관련된 Scene component vector, Scene As관련 data에 대한 호출은 에러를 일으킬 수 있다.
		JComponent::DoActivate();
		impl->RegisterAnimationFrameData();
		impl->OnResourceRef();
		impl->SettingAnimationUpdateData();
		RegisterComponent(impl->thisPointer);
	}
	void JAnimator::DoDeActivate()noexcept
	{
		DeRegisterComponent(impl->thisPointer);
		impl->ClearAnimationUpdateData();
		impl->OffResourceRef();
		impl->DeRegisterAnimationFrameData(); 
		JComponent::DoDeActivate();
	}
	JAnimator::JAnimator(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JAnimatorImpl>(initData, this))
	{ }
	JAnimator::~JAnimator()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimatorPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JAnimatorPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JAnimatorPrivate::AssetDataIOInterface;
	using AnimationUpdateInterface = JAnimatorPrivate::AnimationUpdateInterface;
	using FrameUpdateInterface = JAnimatorPrivate::FrameUpdateInterface;
	using FrameIndexInterface = JAnimatorPrivate::FrameIndexInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{  
		return Core::JPtrUtil::MakeOwnerPtr<JAnimator>(*static_cast<JAnimator::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JAnimator* ani = static_cast<JAnimator*>(createdPtr);
		ani->impl->RegisterThisPointer(ani);
		ani->impl->RegisterPostCreation();
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimator::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JAnimator::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JAnimator::JAnimatorImpl::DoCopy(static_cast<JAnimator*>(from.Get()), static_cast<JAnimator*>(to.Get()));
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)noexcept
	{
		static_cast<JAnimator*>(ptr)->impl->DeRegisterPreDestruction();
		JComponentPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimator::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;
		bool isActivated;

		auto loadData = static_cast<JAnimator::LoadData*>(data);
		JFileIOTool& tool = loadData->tool;
		JUserPtr<JGameObject> owner = loadData->owner;

		JObjectFileIOHelper::LoadComponentIden(tool, guid, flag, isActivated);
		JUserPtr<JAnimationController> aniCont = JObjectFileIOHelper::_LoadHasIden<JAnimationController>(tool, "AnimationController");
		JUserPtr<JSkeletonAsset> skeletonAsset = JObjectFileIOHelper::_LoadHasIden<JSkeletonAsset>(tool, "SkeletonAsset");
		 
		auto idenUser = aPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JAnimator::InitData>(guid, flag, owner), &aPrivate);
		auto aniUser = Core::ConvertChildUserPtr<JAnimator>(std::move(idenUser));
		aniUser->SetAnimatorController(aniCont);
		aniUser->SetSkeletonAsset(skeletonAsset);
		if (!isActivated)
			aniUser->DeActivate();

		return aniUser;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimator::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimator::StoreData*>(data);
		if (!storeData->HasCorrectType(JAnimator::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JAnimator* animator = static_cast<JAnimator*>(storeData->obj.Get());
		JFileIOTool& tool = storeData->tool;

		JObjectFileIOHelper::StoreComponentIden(tool, animator);
		JObjectFileIOHelper::_StoreHasIden(tool, animator->GetAnimatorController().Get(), "AnimationController");
		JObjectFileIOHelper::_StoreHasIden(tool, animator->GetSkeletonAsset().Get(), "SkeletonAsset");

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void AnimationUpdateInterface::OnAnimationUpdate(JUserPtr<JAnimator> ani, Core::JGameTimer* sceneTimer)noexcept
	{
		ani->impl->reqSettingAniData = true;
		ani->impl->userTimer = sceneTimer;
		ani->impl->SettingAnimationUpdateData(); 
	}
	void AnimationUpdateInterface::OffAnimationUpdate(JUserPtr<JAnimator> ani)noexcept
	{
		ani->impl->reqSettingAniData = false;
		ani->impl->userTimer = nullptr;
		ani->impl->ClearAnimationUpdateData();
	}

	bool FrameUpdateInterface::UpdateStart(JAnimator* ani)noexcept
	{ 
		return ani->impl->animationController.IsValid();
	}
	void FrameUpdateInterface::UpdateFrame(JAnimator* ani, Graphic::JAnimationConstants& constant)noexcept
	{
		ani->impl->UpdateFrame(constant);
	}
	void FrameUpdateInterface::UpdateEnd(JAnimator* ani)noexcept
	{
		ani->impl->UpdateFrameEnd();
	}
	int FrameUpdateInterface::GetFrameIndex(JAnimator* ani)noexcept
	{
		return ani->impl->AniFrame::GetFrameIndex();
	} 
	bool FrameUpdateInterface::HasRecopyRequest(JAnimator* ani)noexcept
	{
		return ani->impl->AniFrame::HasMovedDirty();
	}

	int FrameIndexInterface::GetFrameIndex(JAnimator* ani)noexcept
	{
		return ani->impl->AniFrame::GetFrameIndex();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimatorPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	Core::JIdentifierPrivate::DestroyInstanceInterface& JAnimatorPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI; 
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JAnimatorPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}	 
}