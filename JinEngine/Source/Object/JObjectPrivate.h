#pragma once
#include"../Core/Identity/JIdentifierPrivate.h"
#include"../Core/File/JFileIOResult.h"
#include<memory>
#include<string>

namespace JinEngine
{
	class JObject;
	class JObjectPrivate : public Core::JIdentifierPrivate
	{
	public:
		class CreateInstanceInterface : public Core::JIdentifierPrivate::CreateInstanceInterface
		{
		protected:
			void Initialize(Core::JIdentifier* createdPtr, Core::JDITypeDataBase* initData)noexcept override;		//Register This Pointer in impl class
		protected:
			bool CanCopy(JUserPtr<Core::JIdentifier> from, JUserPtr<Core::JIdentifier> to)noexcept override;
		};
		class DestroyInstanceInterface : public Core::JIdentifierPrivate::DestroyInstanceInterface
		{ 
		protected:
			void Clear(Core::JIdentifier* ptr, const bool isForced) override;
		protected:
			bool CanDestroyInstancce(Core::JIdentifier* ptr, const bool isForced)const noexcept override;
		};
	};
}