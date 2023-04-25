#include"JAnimator.h"
#include"JAnimatorPrivate.h" 
#include"../JComponentHint.h"
#include"../../JFrameUpdate.h"
#include"../../Resource/AnimationController/JAnimationController.h"
#include"../../Resource/AnimationController/JAnimationControllerPrivate.h" 
#include"../../Resource/Skeleton/JSkeletonAsset.h"
#include"../../Resource/Skeleton/JSkeletonFixedData.h"
#include"../../Resource/JResourceObject.h" 
#include"../../Resource/JResourceManager.h" 
#include"../../Resource/JResourceObjectUserInterface.h" 
#include"../../GameObject/JGameObject.h" 
#include"../../../Core/FSM/JFSMparameter.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationTime.h"
#include"../../../Core/FSM/AnimationFSM/JAnimationUpdateData.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/File/JFileConstant.h"
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Identity/JIdentifierImplBase.h"
#include"../../../Graphic/FrameResource/JAnimationConstants.h" 
#include<fstream>
namespace JinEngine
{
	namespace
	{
		using ContFrameUpdateInteface = JAnimationControllerPrivate::FrameUpdateInterface;
	}
	namespace
	{
		static auto isAvailableoverlapLam = []() {return false; };
		static JAnimatorPrivate aPrivate;
	} 
 
	class JAnimator::JAnimatorImpl : public Core::JIdentifierImplBase,
		public JResourceObjectUserInterface,
		public JFrameUpdate<JFrameUpdate1<JFrameUpdateBase<Graphic::JAnimationConstants&>>, Core::EmptyType, FrameUpdate::nonBuff>
	{ 
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JAnimatorImpl)
	public:
		JAnimator* thisAnimator;
	public:
		REGISTER_PROPERTY_EX(skeletonAsset, GetSkeletonAsset, SetSkeletonAsset, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		Core::JUserPtr<JSkeletonAsset> skeletonAsset;
		REGISTER_PROPERTY_EX(animationController, GetAnimatorController, SetAnimatorController, GUI_SELECTOR(Core::J_GUI_SELECTOR_IMAGE::NONE, false))
		Core::JUserPtr<JAnimationController> animationController;
		Core::JGameTimer* userTimer = nullptr;
		std::unique_ptr<Core::JAnimationUpdateData> animationUpdateData;
	public:
		bool reqSettingAniData = false;
	public:
		JAnimatorImpl(const InitData& initData, JAnimator* thisAnimator)
			:JResourceObjectUserInterface(thisAnimator->GetGuid()), thisAnimator(thisAnimator)
		{ 
			AddEventListener(*JResourceObject::EvInterface(), thisAnimator->GetGuid(), J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
		}
		~JAnimatorImpl()
		{ 
			RemoveListener(*JResourceObject::EvInterface(), thisAnimator->GetGuid());
		}
	public:
		Core::JUserPtr<JSkeletonAsset>GetSkeletonAsset()const noexcept
		{
			return skeletonAsset;
		}
		Core::JUserPtr<JAnimationController> GetAnimatorController()const noexcept
		{
			return animationController;
		}
	public:
		void SetSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept
		{
			if (thisAnimator->IsActivated())
				CallOffResourceReference(skeletonAsset.Get());
			skeletonAsset = newSkeletonAsset;
			if (thisAnimator->IsActivated())
				CallOnResourceReference(skeletonAsset.Get());
			 
			thisAnimator->ReRegisterComponent();
		}
		void SetAnimatorController(Core::JUserPtr<JAnimationController> newAnimationController)noexcept
		{
			if (thisAnimator->IsActivated())
				CallOffResourceReference(animationController.Get());
			animationController = newAnimationController;
			if (thisAnimator->IsActivated())
				CallOnResourceReference(animationController.Get());

			thisAnimator->ReRegisterComponent();

			//act scene timer
			if (userTimer && thisAnimator->IsActivated())
			{
				ClearAnimationUpdateData();
				reqSettingAniData = true;
				SettingAnimationUpdateData();
			}
		}
		void SettingAnimationUpdateData()noexcept
		{
			if (reqSettingAniData && thisAnimator->IsActivated() && animationController.IsValid())
			{
				animationUpdateData = std::make_unique<Core::JAnimationUpdateData>();
				animationUpdateData->Initialize();
				animationUpdateData->timer = userTimer;
				animationUpdateData->modelSkeleton = skeletonAsset.Get();

				const uint paramCount = animationController->GetParameterCount();
				for (uint i = 0; i < paramCount; ++i)
				{
					Core::JFSMparameter* param = animationController->GetParameterByIndex(i);
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
			animationUpdateData->modelSkeleton = skeletonAsset.Get();
			ContFrameUpdateInteface::Update(animationController.Get(), animationUpdateData.get(), constant);
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
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
		{
			if (iden == thisAnimator->GetGuid())
				return;

			if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
			{
				if (skeletonAsset.IsValid() && skeletonAsset->GetGuid() == jRobj->GetGuid())
					SetSkeletonAsset(Core::JUserPtr<JSkeletonAsset>{});
				else if (animationController.IsValid() && animationController->GetGuid() == jRobj->GetGuid())
					SetAnimatorController(Core::JUserPtr<JAnimationController>{});
			}
		}
	public:
		static void RegisterCallOnce()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JAnimator::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };

			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JGameObject* parent, InitUnq&& parentClassInitData) -> InitUnq
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
			static CTypePrivateFunc cTypeInterfaceFunc{ nullptr, nullptr };
			 
			JComponent::RegisterCTypeInfo(JAnimator::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, cTypeInterfaceFunc);
			Core::JIdentifier::RegisterPrivateInterface(JAnimator::StaticTypeInfo(), aPrivate);
		}
	};

	JAnimator::InitData::InitData(JGameObject* owner)
		:JComponent::InitData(JAnimator::StaticTypeInfo(), owner)
	{}
	JAnimator::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		: JComponent::InitData(JAnimator::StaticTypeInfo(), GetDefaultName(JAnimator::StaticTypeInfo()), guid, flag, owner)
	{}

	Core::JIdentifierPrivate& JAnimator::GetPrivateInterface()const noexcept
	{
		return aPrivate;
	}
	J_COMPONENT_TYPE JAnimator::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	Core::JUserPtr<JSkeletonAsset> JAnimator::GetSkeletonAsset()const noexcept
	{
		return impl->GetSkeletonAsset();
	}
	Core::JUserPtr<JAnimationController> JAnimator::GetAnimatorController()const noexcept
	{
		return impl->GetAnimatorController();
	}
	void JAnimator::SetSkeletonAsset(Core::JUserPtr<JSkeletonAsset> newSkeletonAsset)noexcept
	{
		impl->SetSkeletonAsset(newSkeletonAsset);
	}
	void JAnimator::SetAnimatorController(Core::JUserPtr<JAnimationController> newAnimationController)noexcept
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
		JComponent::DoActivate();
		RegisterComponent();
		impl->OnResourceRef();
		impl->SettingAnimationUpdateData();
	}
	void JAnimator::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent();
		impl->OffResourceRef();
		impl->ClearAnimationUpdateData();
	}
	JAnimator::JAnimator(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JAnimatorImpl>(initData, this))
	{ }
	JAnimator::~JAnimator()
	{
		impl.reset();
	}

	using CreateInstanceInterface = JAnimatorPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JAnimatorPrivate::AssetDataIOInterface;
	using AnimationUpdateInterface = JAnimatorPrivate::AnimationUpdateInterface;
	using FrameUpdateInterface = JAnimatorPrivate::FrameUpdateInterface;

	Core::JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
	{  
		return Core::JPtrUtil::MakeOwnerPtr<JAnimator>(*static_cast<JAnimator::InitData*>(initData.get()));
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JAnimator::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(Core::JIdentifier* from, Core::JIdentifier* to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JAnimator::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JAnimator::JAnimatorImpl::DoCopy(static_cast<JAnimator*>(from), static_cast<JAnimator*>(to));
	}

	Core::JIdentifier* AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimator::LoadData::StaticTypeInfo()))
			return nullptr;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		auto loadData = static_cast<JAnimator::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JGameObject* owner = loadData->owner;

		JFileIOHelper::LoadObjectIden(stream, guid, flag);
		Core::JUserPtr<JAnimationController> aniCont = JFileIOHelper::LoadHasObjectIden<JAnimationController>(stream);
		Core::JUserPtr<JSkeletonAsset> skeletonAsset = JFileIOHelper::LoadHasObjectIden<JSkeletonAsset>(stream);
		 
		auto rawPtr = aPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JAnimator::InitData>(guid, flag, owner), &aPrivate);
		static_cast<JAnimator*>(rawPtr)->SetAnimatorController(aniCont);
		static_cast<JAnimator*>(rawPtr)->SetSkeletonAsset(skeletonAsset);
		return rawPtr;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JAnimator::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JAnimator::StoreData*>(data);
		if (!storeData->HasCorrectType(JAnimator::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JAnimator* animator = static_cast<JAnimator*>(storeData->obj);
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, animator);
		JFileIOHelper::StoreHasObjectIden(stream, animator->GetAnimatorController().Get());
		JFileIOHelper::StoreHasObjectIden(stream, animator->GetSkeletonAsset().Get());

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	void AnimationUpdateInterface::OnAnimationUpdate(JAnimator* ani, Core::JGameTimer* sceneTimer)noexcept
	{
		ani->impl->reqSettingAniData = true;
		ani->impl->userTimer = sceneTimer;
		ani->impl->SettingAnimationUpdateData();
	}
	void AnimationUpdateInterface::OffAnimationUpdate(JAnimator* ani)noexcept
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
		ani->impl->UpdateEnd();
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JAnimatorPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JAnimatorPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}	 
}