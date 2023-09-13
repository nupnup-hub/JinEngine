#include"JComponent.h" 
#include"JComponentPrivate.h"
#include"JComponentHint.h"
#include"../Resource/Scene/JScenePrivate.h"
#include "../GameObject/JGameObject.h"
#include "../GameObject/JGameObjectPrivate.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include<fstream>

namespace JinEngine
{
	class JComponent::JComponentImpl : public Core::JTypeImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JComponentImpl)
	public:
		JWeakPtr<JComponent> thisPointer = nullptr;
	public:
		JUserPtr<JGameObject> owner = nullptr;
	public:
		//For editor redo undo
		std::unique_ptr<Core::JTypeInstanceSearchHint> ownerInfo = nullptr;
	public:
		JComponentImpl(const JComponent::InitData& initData)
			:owner(initData.owner)
		{}
		~JComponentImpl()
		{
			ownerInfo.reset();
		}
	public:
		bool RegisterInstance()noexcept
		{ 
			//For editor redo undo
			if (owner == nullptr && ownerInfo != nullptr)
				owner = Core::GetUserPtr<JGameObject>(*ownerInfo);
			 
			return JGameObjectPrivate::OwnTypeInterface::AddComponent(thisPointer);
		}
		bool DeRegisterInstance()noexcept
		{ 
			ownerInfo = std::make_unique<Core::JTypeInstanceSearchHint>(owner);
			return JGameObjectPrivate::OwnTypeInterface::RemoveComponent(thisPointer);
		}
	public:
		void RegisterThisPointer(JComponent* cPtr)
		{
			thisPointer = Core::GetWeakPtr(cPtr);
		}
		static void RegisterTypeData()
		{
			IMPL_REALLOC_BIND(JComponent::JComponentImpl, thisPointer)
		}
	};

	JComponent::InitData::InitData(const Core::JTypeInfo& typeInfo, const JUserPtr<JGameObject>& owner)
		:JObject::InitData(typeInfo), owner(owner)
	{} 
	JComponent::InitData::InitData(const Core::JTypeInfo& typeInfo, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, const JUserPtr<JGameObject>& owner)
		:JObject::InitData(typeInfo, name, guid, flag), owner(owner)
	{}
	bool JComponent::InitData::IsValidData()const noexcept
	{ 
		return JObject::InitData::IsValidData() && owner != nullptr;
	}

	JComponent::LoadData::LoadData(JUserPtr<JGameObject> owner, std::wifstream& stream, const size_t typeGuid)
		:owner(owner), stream(stream), loadTypeInfo(_JReflectionInfo::Instance().GetTypeInfo(typeGuid))
	{}
	JComponent::LoadData::~LoadData()
	{}
	bool JComponent::LoadData::IsValidData()const noexcept
	{
		return owner != nullptr && stream.is_open() && loadTypeInfo != nullptr;
	}

	JComponent::StoreData::StoreData(JUserPtr<JComponent> comp, std::wofstream& stream)
		:JObject::StoreData(comp), stream(stream)
	{}
	bool JComponent::StoreData::IsValidData()const noexcept
	{
		return JObject::StoreData::IsValidData() && stream.is_open();
	}

	J_OBJECT_TYPE JComponent::GetObjectType()const noexcept
	{
		return J_OBJECT_TYPE::COMPONENT_OBJECT;
	}
	JUserPtr<JGameObject> JComponent::GetOwner()const noexcept
	{
		return impl->owner;
	}
	bool JComponent::PassDefectInspection()const noexcept
	{
		return impl->owner != nullptr;
	}
	void JComponent::DoActivate()noexcept
	{
		JObject::DoActivate();
	}
	void JComponent::DoDeActivate()noexcept
	{
		JObject::DoDeActivate();
	}
	bool JComponent::RegisterComponent(const JUserPtr<JComponent>& comp, UserCompComparePtr comparePtr)noexcept
	{
		return JScenePrivate::CompRegisterInterface::RegisterComponent(comp, comparePtr);
	}
	bool JComponent::DeRegisterComponent(const JUserPtr<JComponent>& comp)noexcept
	{
		return JScenePrivate::CompRegisterInterface::DeRegisterComponent(comp);
	}
	bool JComponent::ReRegisterComponent(const JUserPtr<JComponent>& comp, UserCompComparePtr comparePtr)noexcept
	{
		return JScenePrivate::CompRegisterInterface::ReRegisterComponent(comp, comparePtr);
	}
	void JComponent::RegisterCTypeInfo(const Core::JTypeInfo& typeInfo, const CTypeHint& cTypeHint, const CTypeCommonFunc& cTypeCFunc, const CTypePrivateFunc& cTypePFunc)
	{
		CTypeRegister::RegisterCTypeInfo(typeInfo, cTypeHint, cTypeCFunc, cTypePFunc);
	}
	std::unique_ptr<Core::JDITypeDataBase> JComponent::CreateInitDIData(const J_COMPONENT_TYPE cType, const JTypeInfo& typeInfo, JUserPtr<JGameObject> owner, std::unique_ptr<Core::JDITypeDataBase>&& parentInitData)
	{
		return CTypeCommonCall::CallCreateInitDataCallable(cType, typeInfo, owner, std::move(parentInitData));
	}

	JComponent::JComponent(const InitData& initData) noexcept
		:JObject(initData), impl(std::make_unique<JComponentImpl>(initData))
	{}
	JComponent::~JComponent()
	{ 
		impl.reset(); 
	}

	using CreateInstanceInterface = JComponentPrivate::CreateInstanceInterface;
	using DestroyInstanceInterface = JComponentPrivate::DestroyInstanceInterface;
	using AssetDataIOInterface = JComponentPrivate::AssetDataIOInterface;
	using ActivateInterface = JComponentPrivate::ActivateInterface;

	void CreateInstanceInterface::Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept
	{
		JObjectPrivate::CreateInstanceInterface::Initialize(createdPtr, initData);
		JComponent* comp = static_cast<JComponent*>(createdPtr);
		comp->impl->RegisterThisPointer(comp);
	}
	void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(createdPtr);
		comp->impl->RegisterInstance();	 
	}
	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(createdPtr);
		if (comp->GetOwner()->IsActivated())
			comp->Activate();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced)
	{
		JObjectPrivate::DestroyInstanceInterface::Clear(ptr, isForced);
	}
	void DestroyInstanceInterface::SetInvalidInstance(Core::JIdentifier* ptr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(ptr);
		if (comp->IsActivated())
			comp->DeActivate();  
	} 
	void DestroyInstanceInterface::DeRegisterCash(Core::JIdentifier* ptr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(ptr);
		comp->impl->DeRegisterInstance();
	}

	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(const JUserPtr<JGameObject>& invoker, std::wifstream& stream, const size_t typeGuid)
	{
		return std::make_unique<JComponent::LoadData>(invoker, stream, typeGuid);
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateStoreAssetDIData(const JUserPtr<JComponent>& comp, std::wofstream& stream)
	{
		return std::make_unique<JComponent::StoreData>(comp, stream);
	}

	void ActivateInterface::Activate(const JUserPtr<JComponent>& ptr)noexcept
	{
		ptr->Activate();
	}
	void ActivateInterface::DeActivate(const JUserPtr<JComponent>& ptr)noexcept
	{
		ptr->DeActivate();
	}

	Core::JIdentifierPrivate::DestroyInstanceInterface& JComponentPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}
