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
			using ConvertImplBasePtr = JTypeInfo::ImplTypeInfo::ConvertImplBasePtr;
		public:
			JTypeInfoRegister(const std::string& name)
			{ 
				static JTypeInfo typeInfo{ JTypeInfoInitializer<Type>(name)}; 
			}
		public:
			JTypeInfoRegister(const std::string& implName, const std::string& interfaceName,  JTypeInfo& interfaceClassType, ConvertImplBasePtr convertPtr)
			{ 
				static JTypeInfo typeInfo{ JTypeInfoInitializer<Type>(implName) };
				interfaceClassType.RegisterImplTypeInfo(std::make_unique<JTypeInfo::ImplTypeInfo>(typeInfo, convertPtr));
			}
		};
	}
}