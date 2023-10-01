#include"JIdenCreator.h"
#include"JIdentifierPrivate.h"
#include"JIdentifier.h"
#include"../Guid/JGuidCreator.h"
#include"../Utility/JCommonUtility.h" 

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
	}
}