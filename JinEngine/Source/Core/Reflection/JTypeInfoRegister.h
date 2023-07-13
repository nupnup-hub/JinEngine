#pragma once
#include"JTypeInfo.h" 
#include"JTypeInfoInitializer.h"   

namespace JinEngine
{
	namespace Core
	{
		template<typename Type>
		class JTypeInfoRegister final
		{
		private:
			using ConvertInterfaceBasePtr = JTypeInfo::InterfaceTypeInfo::ConvertInterfacePtr;
			using ConvertImplBasePtr = JTypeInfo::ImplTypeInfo::ConvertImplBasePtr;
		public:
			JTypeInfoRegister(const std::string& name)
			{ 
				static JTypeInfo typeInfo{ JTypeInfoInitializer<Type>(name)}; 
			}
		public:
			JTypeInfoRegister(const std::string& implName, 
				const std::string& interfaceName, 
				JTypeInfo& interfaceClassType, 
				ConvertInterfaceBasePtr convertInterfacePtr,
				ConvertImplBasePtr convertImplPtr)
			{ 
				static JTypeInfo typeInfo{ JTypeInfoInitializer<Type>(implName) };
				interfaceClassType.RegisterImplTypeInfo(std::make_unique<JTypeInfo::ImplTypeInfo>(typeInfo, convertImplPtr));
				typeInfo.RegisterInterfaceTypeInfo(std::make_unique<JTypeInfo::InterfaceTypeInfo>(interfaceClassType, convertInterfacePtr));
			} 
		};
	}
}