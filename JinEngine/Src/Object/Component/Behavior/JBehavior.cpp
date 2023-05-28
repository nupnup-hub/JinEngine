#include"JBehavior.h"  
#include"JBehaviorPrivate.h"
#include"../JComponentHint.h"
#include"../../GameObject/JGameObject.h"  
#include"../../../Core/Guid/GuidCreator.h"
#include"../../../Core/File/JFileIOHelper.h"
#include"../../../Core/Reflection/JTypeImplBase.h"

namespace JinEngine
{
	namespace
	{
		static auto isAvailableoverlapLam = []() {return true; };
		static JBehaviorPrivate bPrivate;
	}
 
	class JBehavior::JBehaviorImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JBehaviorImpl)
	public:
		JWeakPtr<JBehavior> thisPointer;
	public:
		JBehaviorImpl(const InitData& initData, JBehavior* thisBehaviorRaw)
		{}
		~JBehaviorImpl()
		{}
	public:
		static bool DoCopy(JBehavior* from, JBehavior* to)
		{
			//¹Ì±¸Çö
			return true;
		}
	public: 
		void RegisterThisPointer(JBehavior* behav)
		{
			thisPointer = Core::GetWeakPtr(behav);
		}
		static void RegisterTypeData()
		{
			static GetCTypeInfoCallable getTypeInfoCallable{ &JBehavior::StaticTypeInfo };
			static IsAvailableOverlapCallable isAvailableOverlapCallable{ isAvailableoverlapLam };
			using InitUnq = std::unique_ptr<Core::JDITypeDataBase>;
			auto createInitDataLam = [](JUserPtr<JGameObject> parent, InitUnq&& parentClassInitData) -> InitUnq
			{
				using CorrectType = JComponent::ParentType::InitData;
				const bool isValidUnq = parentClassInitData != nullptr && parentClassInitData->GetTypeInfo().IsChildOf(CorrectType::StaticTypeInfo());
				if (isValidUnq)
				{
					CorrectType* ptr = static_cast<CorrectType*>(parentClassInitData.get());
					return std::make_unique<JBehavior::InitData>(ptr->guid, ptr->flag, parent);
				}
				else
					return std::make_unique<JBehavior::InitData>(parent);
			};
			static CreateInitDataCallable createInitDataCallable{ createInitDataLam };

			static CTypeHint cTypeHint{ GetStaticComponentType(), false };
			static CTypeCommonFunc cTypeCommonFunc{ getTypeInfoCallable, isAvailableOverlapCallable, createInitDataCallable };

			JComponent::RegisterCTypeInfo(JBehavior::StaticTypeInfo(), cTypeHint, cTypeCommonFunc, CTypePrivateFunc{});
			Core::JIdentifier::RegisterPrivateInterface(JBehavior::StaticTypeInfo(), bPrivate);

			IMPL_REALLOC_BIND(JBehavior::JBehaviorImpl, thisPointer)
		}
	};

	JBehavior::InitData::InitData(const JUserPtr<JGameObject>& owner)
		:JComponent::InitData(JBehavior::StaticTypeInfo(), owner)
	{}
	JBehavior::InitData::InitData(const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		: JComponent::InitData(JBehavior::StaticTypeInfo(), GetDefaultName(JBehavior::StaticTypeInfo()), guid, flag, owner)
	{}
 
	Core::JIdentifierPrivate& JBehavior::PrivateInterface()const noexcept
	{
		return bPrivate;
	}
	J_COMPONENT_TYPE JBehavior::GetComponentType()const noexcept
	{
		return GetStaticComponentType();
	}
	bool JBehavior::IsAvailableOverlap()const noexcept
	{
		return isAvailableoverlapLam();
	}
	bool JBehavior::PassDefectInspection()const noexcept
	{
		if (JComponent::PassDefectInspection())
			return true;
		else
			return false;
	}
	void JBehavior::DoActivate()noexcept
	{
		JComponent::DoActivate();
		RegisterComponent(impl->thisPointer); 
	}
	void JBehavior::DoDeActivate()noexcept
	{
		JComponent::DoDeActivate();
		DeRegisterComponent(impl->thisPointer);
	}
	JBehavior::JBehavior(const InitData& initData)
		:JComponent(initData), impl(std::make_unique<JBehaviorImpl>(initData, this))
	{}
	JBehavior::~JBehavior() 
	{
		impl.reset();
	}

	using CreateInstanceInterface = JBehaviorPrivate::CreateInstanceInterface;
	using AssetDataIOInterface = JBehaviorPrivate::AssetDataIOInterface;

	JOwnerPtr<Core::JIdentifier> CreateInstanceInterface::Create(Core::JDITypeDataBase* initData)
	{
		return Core::JPtrUtil::MakeOwnerPtr<JBehavior>(*static_cast<JBehavior::InitData*>(initData));
	}
	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JComponentPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JBehavior* behav = static_cast<JBehavior*>(createdPtr);
		behav->impl->RegisterThisPointer(behav);
	}
	bool CreateInstanceInterface::CanCreateInstance(Core::JDITypeDataBase* initData)const noexcept
	{
		const bool isValidPtr = initData != nullptr && initData->GetTypeInfo().IsChildOf(JBehavior::InitData::StaticTypeInfo());
		return isValidPtr && initData->IsValidData();
	}
	bool CreateInstanceInterface::Copy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to) noexcept
	{
		const bool canCopy = CanCopy(from, to) && from->GetTypeInfo().IsA(JBehavior::StaticTypeInfo());
		if (!canCopy)
			return false;

		return JBehavior::JBehaviorImpl::DoCopy(static_cast<JBehavior*>(from.Get()), static_cast<JBehavior*>(to.Get()));
	}

	JUserPtr<Core::JIdentifier> AssetDataIOInterface::LoadAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JBehavior::LoadData::StaticTypeInfo()))
			return nullptr;

		auto loadData = static_cast<JBehavior::LoadData*>(data);
		std::wifstream& stream = loadData->stream;
		JUserPtr<JGameObject> owner = loadData->owner;

		std::wstring guide;
		size_t guid;
		J_OBJECT_FLAG flag;

		JFileIOHelper::LoadObjectIden(stream, guid, flag); 
		auto rawPtr = bPrivate.GetCreateInstanceInterface().BeginCreate(std::make_unique<JBehavior::InitData>(guid, flag, owner), &bPrivate);
		return rawPtr;
	}
	Core::J_FILE_IO_RESULT AssetDataIOInterface::StoreAssetData(Core::JDITypeDataBase* data)
	{
		if (!Core::JDITypeDataBase::IsValidChildData(data, JBehavior::StoreData::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		auto storeData = static_cast<JBehavior::StoreData*>(data);
		if (!storeData->HasCorrectChildType(JBehavior::StaticTypeInfo()))
			return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

		JBehavior* bComp = static_cast<JBehavior*>(storeData->obj.Get());
		std::wofstream& stream = storeData->stream;

		JFileIOHelper::StoreObjectIden(stream, bComp); 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}

	Core::JIdentifierPrivate::CreateInstanceInterface& JBehaviorPrivate::GetCreateInstanceInterface()const noexcept
	{
		static CreateInstanceInterface pI;
		return pI;
	}
	JComponentPrivate::AssetDataIOInterface& JBehaviorPrivate::GetAssetDataIOInterface()const noexcept
	{
		static AssetDataIOInterface pI;
		return pI;
	}

}