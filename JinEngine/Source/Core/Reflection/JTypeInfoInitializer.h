#pragma once
#include"../JCoreEssential.h"
#include<type_traits> 

namespace JinEngine
{
	//class JObject;
	namespace Core
	{
		class JTypeInfo;  
		template<typename Type>
		class JTypeInfoInitializer
		{
		private:
			friend class JTypeInfo;
		private:
			std::string name;
			std::string fullName;
			size_t hashCode;
			JTypeInfo* parent;  
		public:
			JTypeInfoInitializer(const std::string& name)
				:name(name)
			{
				fullName = typeid(Type).name();
				hashCode = std::hash<std::string>{}(fullName);
				if constexpr (std::is_void_v<Type::ParentType>)
					parent = nullptr;
				else
					parent = &Type::ParentType::StaticTypeInfo();
			} 
		};
	}
} 