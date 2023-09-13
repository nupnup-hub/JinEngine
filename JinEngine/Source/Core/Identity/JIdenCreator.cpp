#include"JIdenCreator.h"
#include"JIdentifierPrivate.h"
#include"JIdentifier.h"
#include"../Guid/JGuidCreator.h"
#include"../Utility/JCommonUtility.h"
#include"../../Object/Component/JComponent.h"
#include"../../Object/GameObject/JGameObject.h"
#include"../../Object/Directory/JDirectory.h"

namespace JinEngine
{
	namespace Core
	{
		using CreateInterface = JIdentifierPrivate::CreateInstanceInterface;

		JUserPtr<JIdentifier> JIdenCreatorInterface::Create(std::unique_ptr<JDITypeDataBase>&& initData)
		{
			if (initData == nullptr || !initData->GetTypeInfo().IsChildOf(JIdentifier::InitData::StaticTypeInfo()))
				return nullptr;

			auto rawPtr = static_cast<JIdentifier::InitData*>(initData.get()); 
			auto pInterface = JIdentifier::PrivateInterface(rawPtr->InitDataTypeInfo().TypeGuid());
			return Create(std::move(initData), pInterface);
		}
		JUserPtr<JIdentifier> JIdenCreatorInterface::Create(std::unique_ptr<JDITypeDataBase>&& initData, JIdentifierPrivate* pInterface)
		{
			return ConvertChildUserPtr<JIdentifier>(CreateInterface::BeginCreate(std::move(initData), pInterface));
		}
		JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(std::unique_ptr<JDITypeDataBase>&& initData, JUserPtr<JIdentifier> from)
		{ 
			return ConvertChildUserPtr<JIdentifier>(CreateInterface::BeginCreateAndCopy(std::move(initData), from));
		}
		JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JUserPtr<JGameObject> from, JUserPtr<JGameObject> toParent)
		{ 
			auto initData = std::make_unique<JGameObject::InitData>(from->GetName(), MakeGuid(), OBJECT_FLAG_NONE, toParent);
			return ConvertChildUserPtr<JIdentifier>(CreateInterface::BeginCreateAndCopy(std::move(initData), toParent));
		}
		JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JUserPtr<JComponent> from, JUserPtr<JGameObject> toOwner)
		{
			auto initData = JComponent::CreateInitDIData(from->GetComponentType(), from->GetTypeInfo(), toOwner);
			return ConvertChildUserPtr<JIdentifier>(CreateInterface::BeginCreateAndCopy(std::move(initData), toOwner));
		}
		JUserPtr<JIdentifier> JIdenCreatorInterface::CreateAndCopy(JUserPtr<JDirectory> from, JUserPtr<JDirectory> toParent)
		{
			auto initData = std::make_unique<JDirectory::InitData>(from->GetName(), MakeGuid(), OBJECT_FLAG_NONE, toParent);
			return ConvertChildUserPtr<JIdentifier>(CreateInterface::BeginCreateAndCopy(std::move(initData), toParent));
		}
	}
}