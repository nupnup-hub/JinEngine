#include"JIdenCreator.h"
#include"JIdentifierPrivate.h"
#include"JIdentifier.h"
#include"../Guid/GuidCreator.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Directory/JDirectory.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Core
	{
		using CreateInterface = Core::JIdentifierPrivate::CreateInstanceInterface;

		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData)
		{
			if (initData == nullptr || !initData->GetTypeInfo().IsChildOf(Core::JIdentifier::InitData::StaticTypeInfo()))
				return nullptr;

			auto rawPtr = static_cast<Core::JIdentifier::InitData*>(initData.get()); 
			auto pInterface = JIdentifier::GetPrivateInterface(rawPtr->InitDataTypeInfo().TypeGuid());
			return Create(std::move(initData), pInterface);
		}
		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::Create(std::unique_ptr<Core::JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)
		{
			auto rawPtr = CreateInterface::BeginCreate(std::move(initData), pInterface);
			if (rawPtr != nullptr)
				return Core::GetUserPtr(rawPtr);
			else
				return Core::JUserPtr<JIdentifier>{};
		}
		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(std::unique_ptr<Core::JDITypeDataBase>&& initData, JIdentifier* from)
		{
			auto rawPtr = CreateInterface::BeginCreateAndCopy(std::move(initData), from);
			if (rawPtr != nullptr)
				return Core::GetUserPtr(rawPtr);
			else
				return Core::JUserPtr<JIdentifier>{};
		}
		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JGameObject* from, JGameObject* toParent)
		{  
			auto initData = std::make_unique<JGameObject::InitData>(from->GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, toParent);
			auto result = CreateInterface::BeginCreateAndCopy(std::move(initData), toParent);
			return Core::GetUserPtr(result);
		}
		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JComponent* from, JGameObject* toOwner)
		{
			auto initData = JComponent::CreateInitDIData(from->GetComponentType(), toOwner);
			auto result = CreateInterface::BeginCreateAndCopy(std::move(initData), toOwner);
			return Core::GetUserPtr(result);
		}
		Core::JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JDirectory* from, JDirectory* toParent)
		{
			auto initData = std::make_unique<JDirectory::InitData>(from->GetName(), Core::MakeGuid(), OBJECT_FLAG_NONE, toParent);
			auto result = CreateInterface::BeginCreateAndCopy(std::move(initData), toParent);
			return Core::GetUserPtr(result);
		}
	}
}