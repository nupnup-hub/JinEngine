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