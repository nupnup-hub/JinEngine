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
			bool CanCopy(Core::JIdentifier* from, Core::JIdentifier* to)noexcept override;
		};
		class DestroyInstanceInterface : public Core::JIdentifierPrivate::DestroyInstanceInterface
		{ 
		protected:
			bool CanDestroyInstancce(Core::JIdentifier* ptr, const bool isForced)const noexcept override; 
		};
	};
}