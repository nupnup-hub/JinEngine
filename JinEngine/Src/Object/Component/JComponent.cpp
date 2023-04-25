#include"JComponent.h" 
#include"JComponentPrivate.h"
#include"JComponentHint.h"
#include"../Resource/Scene/JScenePrivate.h"
#include "../GameObject/JGameObject.h"
#include "../GameObject/JGameObjectPrivate.h"
#include"../../Utility/JCommonUtility.h"
#include"../../Core/Identity/JIdentifierImplBase.h"
#include<fstream>

namespace JinEngine
{
	class JComponent::JComponentImpl : public Core::JIdentifierImplBase
	{
		REGISTER_CLASS_IDENTIFIER_LINE_IMPL(JComponentImpl)
	public:
		JGameObject* owner = nullptr;
	public:
		//For editor redo undo
		std::unique_ptr<Core::JTypeInstanceSearchHint> ownerInfo;
	public:
		JComponentImpl(const JComponent::InitData& initData)
			:owner(initData.owner)
		{}
		~JComponentImpl()
		{}
	public:
		bool RegisterInstance(JComponent* comp)noexcept
		{ 
			//For editor redo undo
			if (owner == nullptr && ownerInfo != nullptr)
				owner = static_cast<JGameObject*>(Core::GetRawPtr(*ownerInfo));
			 
			return JGameObjectPrivate::OwnTypeInterface::AddComponent(comp);
		}
		bool DeRegisterInstance(JComponent* comp)noexcept
		{
			ownerInfo = std::make_unique<Core::JTypeInstanceSearchHint>(Core::GetUserPtr(owner));
			return JGameObjectPrivate::OwnTypeInterface::RemoveComponent(comp);
		}
	};

	JComponent::InitData::InitData(const Core::JTypeInfo& typeInfo, JGameObject* owner)
		:JObject::InitData(typeInfo), owner(owner)
	{} 
	JComponent::InitData::InitData(const Core::JTypeInfo& typeInfo, const std::wstring& name, const size_t guid, const J_OBJECT_FLAG flag, JGameObject* owner)
		:JObject::InitData(typeInfo, name, guid, flag), owner(owner)
	{}
	bool JComponent::InitData::IsValidData()const noexcept
	{ 
		return JObject::InitData::IsValidData() && owner != nullptr;
	}

	JComponent::LoadData::LoadData(JGameObject* owner, std::wifstream& stream)
		:owner(owner), stream(stream)
	{}
	JComponent::LoadData::~LoadData()
	{}
	bool JComponent::LoadData::IsValidData()const noexcept
	{
		return owner != nullptr && stream.is_open();
	}

	JComponent::StoreData::StoreData(JComponent* comp, std::wofstream& stream)
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
	JGameObject* JComponent::GetOwner()const noexcept
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
	bool JComponent::RegisterComponent()noexcept
	{
		return JScenePrivate::CompRegisterInterface::RegisterComponent(this);
	}
	bool JComponent::DeRegisterComponent()noexcept
	{
		return JScenePrivate::CompRegisterInterface::DeRegisterComponent(this);
	}
	bool JComponent::ReRegisterComponent()noexcept
	{
		return JScenePrivate::CompRegisterInterface::ReRegisterComponent(this);
	}
	void JComponent::RegisterCTypeInfo(const Core::JTypeInfo& typeInfo, const CTypeHint& cTypeHint, const CTypeCommonFunc& cTypeCFunc, const CTypePrivateFunc& cTypePFunc)
	{
		CTypeRegister::RegisterCTypeInfo(typeInfo, cTypeHint, cTypeCFunc, cTypePFunc);
	}
	std::unique_ptr<Core::JDITypeDataBase> JComponent::CreateInitDIData(const J_COMPONENT_TYPE cType, JGameObject* parent, std::unique_ptr<Core::JDITypeDataBase>&& parentClassInitData)
	{
		return CTypeCommonCall::CallCreateInitDataCallable(cType, parent, std::move(parentClassInitData));
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

	void CreateInstanceInterface::RegisterCash(Core::JIdentifier* createdPtr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(createdPtr);
		comp->impl->RegisterInstance(comp);
	}
	void CreateInstanceInterface::SetValidInstance(Core::JIdentifier* createdPtr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(createdPtr);
		if (comp->GetOwner()->IsActivated())
			comp->Activate();
	}

	void DestroyInstanceInterface::Clear(Core::JIdentifier* ptr, const bool isForced){}
	void DestroyInstanceInterface::SetInvalidInstance(Core::JIdentifier* ptr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(ptr);
		if (comp->IsActivated())
			comp->DeActivate(); 
	} 
	void DestroyInstanceInterface::DeRegisterCash(Core::JIdentifier* ptr)noexcept
	{
		JComponent* comp = static_cast<JComponent*>(ptr);
		comp->impl->DeRegisterInstance(comp);
	}

	void ActivateInterface::Activate(JComponent* ptr)noexcept
	{
		ptr->Activate();
	}
	void ActivateInterface::DeActivate(JComponent* ptr)noexcept
	{
		ptr->DeActivate();
	}

	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateLoadAssetDIData(JGameObject* invoker, std::wifstream& stream)
	{
		return std::make_unique<JComponent::LoadData>(invoker, stream);
	}
	std::unique_ptr<Core::JDITypeDataBase> AssetDataIOInterface::CreateStoreAssetDIData(JComponent* comp, std::wofstream& stream)
	{
		return std::make_unique<JComponent::StoreData>(comp, stream);
	}

	Core::JIdentifierPrivate::DestroyInstanceInterface& JComponentPrivate::GetDestroyInstanceInterface()const noexcept
	{
		static DestroyInstanceInterface pI;
		return pI;
	}
}
