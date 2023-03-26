#pragma once
#include<type_traits>
#include<string> 

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
				hashCode = typeid(Type).hash_code();
				if constexpr (std::is_void_v<Type::ParentType>)
					parent = nullptr;
				else
					parent = &Type::ParentType::StaticTypeInfo();
			} 
		};
	}
} 